#ifndef ENABLE_TEMPORAL_AA
#define ENABLE_TEMPORAL_AA  1
#endif

uniform sampler2D texMain;
uniform sampler2D texPrev;
uniform sampler2D texNoise;
uniform sampler2D texDepth;
uniform mat4 argViewProj;
uniform mat4 argPrevViewProj;
uniform vec2 viewportSize;

in vec2 v_texCoord;

out vec4 OutColor;

uniform float SampleWeights[9];
uniform float LowpassWeights[9];
uniform float PlusWeights[5];
uniform float VelocityScaling;

vec3 RGBToYCoCg( vec3 RGB )
{
    float Y  = dot( RGB, vec3(  1, 2,  1 ) );
    float Co = dot( RGB, vec3(  2, 0, -2 ) );
    float Cg = dot( RGB, vec3( -1, 2, -1 ) );

    vec3 YCoCg = vec3( Y, Co, Cg );
    return YCoCg;
}

vec3 YCoCgToRGB( vec3 YCoCg )
{
    float Y  = YCoCg.x * 0.25;
    float Co = YCoCg.y * 0.25;
    float Cg = YCoCg.z * 0.25;

    float R = Y + Co - Cg;
    float G = Y + Cg;
    float B = Y - Co - Cg;

    vec3 RGB = vec3( R, G, B );
    return RGB;
}

// Faster but less accurate luma computation.
// Luma includes a scaling by 4.
float Luma4(vec3 Color)
{
    return (Color.g * 2.0) + (Color.r + Color.b);
}

// Optimized HDR weighting function.
float HdrWeight4(vec3 Color, float Exposure)
{
    return 1.0 / (Luma4(Color) * Exposure + 4.0);
}

float HdrWeightY(float Color, float Exposure)
{
    return 1.0 / (Color * Exposure + 1.0);
}

float HdrWeightG(vec3 Color, float Exposure)
{
    return 1.0 / (Color.g * Exposure + 1.0);
}

float HdrWeightG_(float Color, float Exposure)
{
    return 1.0 / (Color * Exposure + 1.0);
}

// Optimized HDR weighting function.
float HdrWeight4_(float Color, float Exposure)
{
    return 1.0 / (Color * Exposure + 4.0);
}

// Optimized HDR weighting inverse.
float HdrWeightInv4(vec3 Color, float Exposure)
{
    return 4.0 * (1.0 / (Luma4(Color) * (-Exposure) + 1.0));
}

float HdrWeightInvG(vec3 Color, float Exposure)
{
    return 1.0 / (Color.g * (-Exposure) + 1.0);
}

float HdrWeightInvY(float Color, float Exposure)
{
    return 1.0 / (Color * (-Exposure) + 1.0);
}

float HdrWeightInv4_(float Color, float Exposure)
{
    return 4.0 * (1.0 / (Color * (-Exposure) + 1.0));
}

float HdrWeightInvG_(float Color, float Exposure)
{
    return 1.0 / (Color * (-Exposure) + 1.0);
}

// This returns exposure normalized linear luma from a PerceptualLuma4().
float LinearLuma4(float Channel, float Exposure)
{
    return Channel * HdrWeightInv4_(Channel, Exposure);
}

// This returns exposure normalized linear luma from a PerceptualLuma4().
float LinearLumaG(float Channel, float Exposure)
{
    return Channel * HdrWeightInvG_(Channel, Exposure);
}

float PerceptualLuma4(vec3 Color, float Exposure)
{
    float L = Luma4(Color);
    return L * HdrWeight4_(L, Exposure);
}

float PerceptualLumaG(vec3 Color, float Exposure)
{
    return Color.g * HdrWeightG_(Color.g, Exposure);
}

float Luma(vec3 Color)
{
    #if 1
        // This seems to work better (less same luma ghost trails).
        // CCIR 601 function for luma.
        return dot(Color, vec3(0.299, 0.587, 0.114));
    #else
        // Rec 709 function for luma.
        return dot(Color, vec3(0.2126, 0.7152, 0.0722));
    #endif
}

float HighlightCompression(float Channel)
{
    return Channel * (1.0 / (1.0 + Channel));
}

float HighlightDecompression(float Channel)
{
    return Channel * (1.0 / (1.0 - Channel));
}

float PerceptualLuma(vec3 Color, float Exposure)
{
    return sqrt(HighlightCompression(Luma(Color) * Exposure));
}

float LinearLuma(float Channel)
{
    // This returns exposure normalized linear luma from a PerceptualLuma().
    return HighlightDecompression(Channel * Channel);
}

// Intersect ray with AABB, knowing there is an intersection.
//   Dir = Ray direction.
//   Org = Start of the ray.
//   Box = Box is at {0,0,0} with this size.
// Returns distance on line segment.
float IntersectAABB(vec3 Dir, vec3 Org, vec3 Box)
{
    #if PS4_PROFILE
        // This causes flicker, it should only be used on PS4 until proper fix is in.
        if(min(min(abs(Dir.x), abs(Dir.y)), abs(Dir.z)) < (1.0/65536.0)) return 1.0;
    #endif
    vec3 RcpDir = 1.0 / Dir;
    vec3 TNeg = (  Box  - Org) * RcpDir;
    vec3 TPos = ((-Box) - Org) * RcpDir;
    return max(max(min(TNeg.x, TPos.x), min(TNeg.y, TPos.y)), min(TNeg.z, TPos.z));
}

float HistoryClamp(vec3 History, vec3 Filtered, vec3 NeighborMin, vec3 NeighborMax)
{
    vec3 Min = min(Filtered, min(NeighborMin, NeighborMax));
    vec3 Max = max(Filtered, max(NeighborMin, NeighborMax));
    vec3 Avg2 = Max + Min;
    vec3 Dir = Filtered - History;
    vec3 Org = History - Avg2 * 0.5;
    vec3 Scale = Max - Avg2 * 0.5;
    return clamp(IntersectAABB(Dir, Org, Scale), 0.0, 1.0);
}

float HdrWeight(vec3 Color, float Exposure)
{
    return 1.0 / (max(Luma(Color) * Exposure, 1.0));
}

vec4 HdrLerp(vec4 ColorA, vec4 ColorB, float Blend, float Exposure)
{
    float BlendA = (1.0 - Blend) * HdrWeight(ColorA.rgb, Exposure);
    float BlendB =        Blend  * HdrWeight(ColorB.rgb, Exposure);
    float RcpBlend = 1.0 / (BlendA + BlendB);
    BlendA *= RcpBlend;
    BlendB *= RcpBlend;
    return ColorA * BlendA + ColorB * BlendB;
}

void Bicubic2DCatmullRom( in vec2 UV, in vec2 Size, out vec2 Sample[3], out vec2 Weight[3] )
{
    vec2 InvSize = 1.0 / Size;

    UV *= Size;

    vec2 tc = floor( UV - 0.5 ) + 0.5;
    vec2 f = UV - tc;
    vec2 f2 = f * f;
    vec2 f3 = f2 * f;

    vec2 w0 = f2 - 0.5 * (f3 + f);
    vec2 w1 = 1.5 * f3 - 2.5 * f2 + 1;
    vec2 w3 = 0.5 * (f3 - f2);
    vec2 w2 = 1 - w0 - w1 - w3;

    Weight[0] = w0;
    Weight[1] = w1 + w2;
    Weight[2] = w3;

    Sample[0] = tc - 1;
    Sample[1] = tc + w2 / Weight[1];
    Sample[2] = tc + 2;

    Sample[0] *= InvSize;
    Sample[1] *= InvSize;
    Sample[2] *= InvSize;
}

#define AA_FILTERED 1
#define AA_BORDER 1
#define AA_ALPHA 0
#define AA_CROSS 2
//#define AA_CROSS 0
#define AA_GREEN_AS_LUMA 1
#define AA_AABB 1
#define AA_LOWPASS 0
#define AA_DEBUG 0
#define AA_VELOCITY_WEIGHTING 0
#define AA_YCOCG 1
#define AA_BICUBIC 1

// 0 / 1(default) This can be set to 0 to debug the velocity vectors
#define NEIGHBORHOOD_CLAMPING 1

// 1 = Use tighter AABB clamp for history.
// 0 = Use simple min/max clamp.
#ifndef AA_AABB
    #define AA_AABB 1
#endif

// 0 = Anti-alias the alpha channel also (not getting used currently).
// 1 = Use alpha channel to improve quality (required for primary AA).
//     Leverages dead code removal to work in RGB instead of RGBA.
#ifndef AA_ALPHA
    #define AA_ALPHA 1
#endif

// Cross distance in pixels used in depth search X pattern.
// 0 = Turn this feature off.
// 2 = Is required for standard temporal AA pass.
#ifndef AA_CROSS
    #define AA_CROSS 2
#endif

// 1 = Render in blue, with green = diff between frames, red = alpha channel.
// 0 = Non-debug.
#ifndef AA_DEBUG
    #define AA_DEBUG 0
#endif

// 2 = Dilate in cross pattern by 2 pixels in distance (this can be larger than 2 if required).
// 1 = Dilate history alpha using maximum of neighborhood.
//     This increases thin edge quality in motion.
//     This is only valid for AA_ALPHA == 1
// 0 = Turn off.
#ifndef AA_DILATE
    #define AA_DILATE AA_ALPHA
#endif

// 1 = Use dynamic motion.
// 0 = Skip dynamic motion, currently required for half resolution passes.
#ifndef AA_DYNAMIC
    #define AA_DYNAMIC 1
#endif

// 1 = Use filtered sample.
// 0 = Use center sample.
#ifndef AA_FILTERED
    #define AA_FILTERED 1
#endif

// 0 = Dynamic motion based lerp value (default).
// non-zero = Use 1/LERP fixed lerp value (used for reflections).
#ifndef AA_LERP
    #define AA_LERP 0
#endif

// 1 = Use extra lowpass filter for quality bump.
// 0 = Don't use.
#ifndef AA_LOWPASS
    #define AA_LOWPASS 1
#endif

// 1 = Use higher quality round clamp.
// 0 = Use lower quality but faster box clamp.
#ifndef AA_ROUND
    #define AA_ROUND 1
#endif

// 1 = Use extra clamp to avoid NANs
// 0 = Don't use.
#ifndef AA_NAN
    #define AA_NAN 1
#endif

// Fix for lack of borders during current frame filter.
#ifndef AA_BORDER
    #define AA_BORDER 0
#endif

// Force clamp on alpha.
#ifndef AA_FORCE_ALPHA_CLAMP
    #define AA_FORCE_ALPHA_CLAMP 0
#endif

// Use YCoCg path.
#ifndef AA_YCOCG
    #define AA_YCOCG 0
#endif

// Use green as luma.
#ifndef AA_GREEN_AS_LUMA
    #define AA_GREEN_AS_LUMA AA_YCOCG
#endif

// Bicubic filter history
#ifndef AA_BICUBIC
    #define AA_BICUBIC 0
#endif

// Special adjustments for DOF.
#ifndef AA_DOF
    #define AA_DOF 0
#endif

// Tone map to kill fireflies
#ifndef AA_TONE
    #define AA_TONE 1
#endif

// Neighborhood clamping. Disable for testing reprojection.
#ifndef AA_CLAMP
    #define AA_CLAMP 1
#endif

// Antighosting using dynamic mask
#ifndef AA_DYNAMIC_ANTIGHOST
    #define AA_DYNAMIC_ANTIGHOST 0
#endif

void main()
{
    float InExposureScale = 1.0;

    vec2 UV = v_texCoord;

    vec4 ViewportSize = vec4(viewportSize.x, viewportSize.y, 1.0 / viewportSize.x, 1.0 / viewportSize.y);
    vec4 ScreenPosToPixel = vec4(viewportSize.x * 0.5, viewportSize.y * 0.5, viewportSize.x * 0.5 - 0.5, viewportSize.y * 0.5 - 0.5);
    vec2 texMainSize = vec2(textureSize(texMain, 0));
    vec4 PostprocessInput0Size = vec4(texMainSize.x, texMainSize.y, 1.0 / texMainSize.x, 1.0 / texMainSize.y);
    vec4 PostprocessInput1Size = PostprocessInput0Size;

    vec2 ScreenPos = ( UV * PostprocessInput0Size.xy - 0.5 - ScreenPosToPixel.zw ) / ScreenPosToPixel.xy;

    // FIND MOTION OF PIXEL AND NEAREST IN NEIGHBORHOOD
    // ------------------------------------------------
    vec3 PosN; // Position of this pixel, possibly later nearest pixel in neighborhood.
    PosN.xy = ScreenPos;
    PosN.z = texture(texDepth, UV).r;
    // Screen position of minimum depth.
    vec2 VelocityOffset = vec2(0.0, 0.0);
    #if AA_CROSS
        // For motion vector, use camera/dynamic motion from min depth pixel in pattern around pixel.
        // This enables better quality outline on foreground against different motion background.
        // Larger 2 pixel distance "x" works best (because AA dilates surface).
        vec4 Depths;
        Depths.x = 1.0 - textureLodOffset(texDepth, UV, 0, ivec2(-AA_CROSS, -AA_CROSS)).r;
        Depths.y = 1.0 - textureLodOffset(texDepth, UV, 0, ivec2( AA_CROSS, -AA_CROSS)).r;
        Depths.z = 1.0 - textureLodOffset(texDepth, UV, 0, ivec2(-AA_CROSS,  AA_CROSS)).r;
        Depths.w = 1.0 - textureLodOffset(texDepth, UV, 0, ivec2( AA_CROSS,  AA_CROSS)).r;

        vec2 DepthOffset = vec2(AA_CROSS, AA_CROSS);
        float DepthOffsetXx = float(AA_CROSS);

        // Nearest depth is the largest depth (depth surface 0=far, 1=near).
        if(Depths.x > Depths.y)
        {
            DepthOffsetXx = -AA_CROSS;
        }
        if(Depths.z > Depths.w)
        {
            DepthOffset.x = -AA_CROSS;
        }
        float DepthsXY = max(Depths.x, Depths.y);
        float DepthsZW = max(Depths.z, Depths.w);
        if(DepthsXY > DepthsZW)
        {
            DepthOffset.y = -AA_CROSS;
            DepthOffset.x = DepthOffsetXx;
        }
        float DepthsXYZW = max(DepthsXY, DepthsZW);
        if(DepthsXYZW > (1.0 - PosN.z))
        {
            // This is offset for reading from velocity texture.
            // This supports half or fractional resolution velocity textures.
            // With the assumption that UV position scales between velocity and color.
            VelocityOffset = DepthOffset * PostprocessInput0Size.zw;
            // This is [0 to 1] flipped in Y.
            //PosN.xy = ScreenPos + DepthOffset * ViewportSize.zw * 2.0;
            PosN.z = 1.0 - DepthsXYZW;
        }
    #endif  // AA_CROSS

    vec2 AN = (PosN.xy * ScreenPosToPixel.xy + ScreenPosToPixel.zw + 0.5) * PostprocessInput0Size.zw;
    vec4 WSP = vec4(AN * 2.f - 1.f, PosN.z * 2.0 - 1.0, 1.f) * inverse(argViewProj);
    WSP /= WSP.w;
    vec4 CVVPosHISTORY = WSP * argPrevViewProj;
    vec2 UVHISTORY = 0.5 * (CVVPosHISTORY.xy / CVVPosHISTORY.w) + 0.5;

    vec2 PrevScreen = ( UVHISTORY * PostprocessInput0Size.xy - 0.5 - ScreenPosToPixel.zw ) / ScreenPosToPixel.xy;
    vec2 BackN = PosN.xy - PrevScreen;
    //vec2 BackN = (UV - UVHISTORY) * 2.0;

    vec2 BackTemp = BackN * ViewportSize.xy;
    #if AA_DYNAMIC
        vec2 VelocityN;
        #if AA_CROSS
            VelocityN = texture(texNoise, UV + VelocityOffset).xy;
        #else
            VelocityN = texture(texNoise, UV).xy;
        #endif
        VelocityN *= VelocityScaling;
        bool DynamicN = VelocityN.x < 65534.0;
        if(DynamicN)
        {
            BackN = VelocityN;
        }
        BackTemp = BackN * ViewportSize.xy;
    #endif
    #if !AA_BICUBIC
        // Save the amount of pixel offset of just camera motion, used later as the amount of blur introduced by history.
        float HistoryBlurAmp = 2.0;
        float HistoryBlur = saturate(abs(BackTemp.x) * HistoryBlurAmp + abs(BackTemp.y) * HistoryBlurAmp);
        float Velocity = sqrt(dot(BackTemp, BackTemp));
    #endif
    // Easier to do off screen check before conversion.
    // BackN is in units of 2pixels/viewportWidthInPixels
    // This converts back projection vector to [-1 to 1] offset in viewport.
    BackN = ScreenPos - BackN;
    bool OffScreen = max(abs(BackN.x), abs(BackN.y)) >= 1.0;
    // Also clamp to be on screen (fixes problem with DOF).
    // The .z and .w is the 1/width and 1/height.
    // This clamps to be a pixel inside the viewport.
    BackN.x = clamp(BackN.x, -1.0 + ViewportSize.z, 1.0 - ViewportSize.z);
    BackN.y = clamp(BackN.y, -1.0 + ViewportSize.w, 1.0 - ViewportSize.w);
    // Convert from [-1 to 1] to view rectangle which is somewhere in [0 to 1].
    // The extra +0.5 factor is because ScreenPosToPixel.zw is incorrectly computed
    // as the upper left of the pixel instead of the center of the pixel.
    BackN = (BackN * ScreenPosToPixel.xy + ScreenPosToPixel.zw + 0.5) * PostprocessInput0Size.zw;



    // FILTER PIXEL (RESAMPLE TO REMOVE JITTER OFFSET) AND GET NEIGHBORHOOD
    // --------------------------------------------------------------------
    // 012
    // 345
    // 678
    #if AA_YCOCG
        // Special case, only using 5 taps.
        vec4 Neighbor1 = textureLodOffset(texMain, UV, 0.0, ivec2(0, -1));
        vec4 Neighbor3 = textureLodOffset(texMain, UV, 0, ivec2(-1,  0));
        vec4 Neighbor4 = texture(texMain, UV);
        vec4 Neighbor5 = textureLodOffset(texMain, UV, 0, ivec2( 1,  0));
        vec4 Neighbor7 = textureLodOffset(texMain, UV, 0, ivec2( 0,  1));
        Neighbor1.rgb = RGBToYCoCg(Neighbor1.rgb);
        Neighbor3.rgb = RGBToYCoCg(Neighbor3.rgb);
        Neighbor4.rgb = RGBToYCoCg(Neighbor4.rgb);
        Neighbor5.rgb = RGBToYCoCg(Neighbor5.rgb);
        Neighbor7.rgb = RGBToYCoCg(Neighbor7.rgb);
        #if AA_TONE
            Neighbor1.xyz *= HdrWeightY(Neighbor1.x, InExposureScale);
            Neighbor3.xyz *= HdrWeightY(Neighbor3.x, InExposureScale);
            Neighbor4.xyz *= HdrWeightY(Neighbor4.x, InExposureScale);
            Neighbor5.xyz *= HdrWeightY(Neighbor5.x, InExposureScale);
            Neighbor7.xyz *= HdrWeightY(Neighbor7.x, InExposureScale);
        #endif
        #if AA_FILTERED
            vec4 Filtered =
                Neighbor1 * PlusWeights[0] +
                Neighbor3 * PlusWeights[1] +
                Neighbor4 * PlusWeights[2] +
                Neighbor5 * PlusWeights[3] +
                Neighbor7 * PlusWeights[4];
            #if AA_BORDER
                // Use unfiltered for 1 pixel border.
                vec2 TestPos = abs(ScreenPos);
                // Add 1 pixel and check if off screen.
                TestPos += ViewportSize.zw * 2.0;
                bool FilteredOffScreen = max(TestPos.x, TestPos.y) >= 1.0;
                if(FilteredOffScreen)
                {
                    Filtered = Neighbor4;
                }
            #endif
        #else
            // Unfiltered.
            vec4 Filtered = Neighbor4;
        #endif
        vec4 FilteredLow = Filtered;
#if 1
        // Neighborhood seems to only need the "+" pattern.
        vec4 NeighborMin = min(min(min(Neighbor1, Neighbor3), min(Neighbor4, Neighbor5)), Neighbor7);
        vec4 NeighborMax = max(max(max(Neighbor1, Neighbor3), max(Neighbor4, Neighbor5)), Neighbor7);
#else
        vec4 m1, m2;
        m1  = Neighbor1; m2  = Neighbor1 * Neighbor1;
        m1 += Neighbor3; m2 += Neighbor3 * Neighbor3;
        m1 += Neighbor4; m2 += Neighbor4 * Neighbor4;
        m1 += Neighbor5; m2 += Neighbor5 * Neighbor5;
        m1 += Neighbor7; m2 += Neighbor7 * Neighbor7;

        vec4 mu = m1 * (1.0 / 5.0);
        vec4 Sigma = sqrt( abs(m2 * (1.0 / 5.0) - mu * mu) );
        vec4 NeighborMin = mu - 1.0 * Sigma;
        vec4 NeighborMax = mu + 1.0 * Sigma;
#endif
    #else
        float4 Neighbor0 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2(-1, -1));
        float4 Neighbor1 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2( 0, -1));
        float4 Neighbor2 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2( 1, -1));
        float4 Neighbor3 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2(-1,  0));
        float4 Neighbor4 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0);
        float4 Neighbor5 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2( 1,  0));
        float4 Neighbor6 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2(-1,  1));
        float4 Neighbor7 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2( 0,  1));
        float4 Neighbor8 = PostprocessInput0.SampleLevel(PostprocessInput0Sampler, UV, 0, int2( 1,  1));
        #if AA_GREEN_AS_LUMA
            Neighbor0.rgb *= HdrWeightG(Neighbor0.rgb, InExposureScale);
            Neighbor1.rgb *= HdrWeightG(Neighbor1.rgb, InExposureScale);
            Neighbor2.rgb *= HdrWeightG(Neighbor2.rgb, InExposureScale);
            Neighbor3.rgb *= HdrWeightG(Neighbor3.rgb, InExposureScale);
            Neighbor4.rgb *= HdrWeightG(Neighbor4.rgb, InExposureScale);
            Neighbor5.rgb *= HdrWeightG(Neighbor5.rgb, InExposureScale);
            Neighbor6.rgb *= HdrWeightG(Neighbor6.rgb, InExposureScale);
            Neighbor7.rgb *= HdrWeightG(Neighbor7.rgb, InExposureScale);
            Neighbor8.rgb *= HdrWeightG(Neighbor8.rgb, InExposureScale);
        #else
            Neighbor0.rgb *= HdrWeight4(Neighbor0.rgb, InExposureScale);
            Neighbor1.rgb *= HdrWeight4(Neighbor1.rgb, InExposureScale);
            Neighbor2.rgb *= HdrWeight4(Neighbor2.rgb, InExposureScale);
            Neighbor3.rgb *= HdrWeight4(Neighbor3.rgb, InExposureScale);
            Neighbor4.rgb *= HdrWeight4(Neighbor4.rgb, InExposureScale);
            Neighbor5.rgb *= HdrWeight4(Neighbor5.rgb, InExposureScale);
            Neighbor6.rgb *= HdrWeight4(Neighbor6.rgb, InExposureScale);
            Neighbor7.rgb *= HdrWeight4(Neighbor7.rgb, InExposureScale);
            Neighbor8.rgb *= HdrWeight4(Neighbor8.rgb, InExposureScale);
        #endif
        #if AA_FILTERED
            float4 Filtered =
                Neighbor0 * SampleWeights[0] +
                Neighbor1 * SampleWeights[1] +
                Neighbor2 * SampleWeights[2] +
                Neighbor3 * SampleWeights[3] +
                Neighbor4 * SampleWeights[4] +
                Neighbor5 * SampleWeights[5] +
                Neighbor6 * SampleWeights[6] +
                Neighbor7 * SampleWeights[7] +
                Neighbor8 * SampleWeights[8];
            #if AA_LOWPASS
                float4 FilteredLow =
                    Neighbor0 * LowpassWeights[0] +
                    Neighbor1 * LowpassWeights[1] +
                    Neighbor2 * LowpassWeights[2] +
                    Neighbor3 * LowpassWeights[3] +
                    Neighbor4 * LowpassWeights[4] +
                    Neighbor5 * LowpassWeights[5] +
                    Neighbor6 * LowpassWeights[6] +
                    Neighbor7 * LowpassWeights[7] +
                    Neighbor8 * LowpassWeights[8];
            #else
                float4 FilteredLow = Filtered;
            #endif
            #if AA_BORDER
                // Use unfiltered for 1 pixel border.
                float2 TestPos = abs(ScreenPos);
                // Add 1 pixel and check if off screen.
                TestPos += ViewportSize.zw * 2.0;
                bool FilteredOffScreen = max(TestPos.x, TestPos.y) >= 1.0;
                if(FilteredOffScreen)
                {
                    Filtered = Neighbor4;
                    FilteredLow = Neighbor4;
                }
            #endif
        #else
            // Unfiltered.
            float4 Filtered = Neighbor4;
            float4 FilteredLow = Neighbor4;
        #endif
        #if AA_ROUND
            float4 NeighborMin2 = min(min(Neighbor0, Neighbor2), min(Neighbor6, Neighbor8));
            float4 NeighborMax2 = max(max(Neighbor0, Neighbor2), max(Neighbor6, Neighbor8));
            float4 NeighborMin = min(min(min(Neighbor1, Neighbor3), min(Neighbor4, Neighbor5)), Neighbor7);
            float4 NeighborMax = max(max(max(Neighbor1, Neighbor3), max(Neighbor4, Neighbor5)), Neighbor7);
            NeighborMin2 = min(NeighborMin2, NeighborMin);
            NeighborMax2 = max(NeighborMax2, NeighborMax);
            NeighborMin = NeighborMin * 0.5 + NeighborMin2 * 0.5;
            NeighborMax = NeighborMax * 0.5 + NeighborMax2 * 0.5;

            float4 m1, m2;
            m1  = Neighbor0; m2  = Neighbor0 * Neighbor0;
            m1 += Neighbor1; m2 += Neighbor1 * Neighbor1;
            m1 += Neighbor2; m2 += Neighbor2 * Neighbor2;
            m1 += Neighbor3; m2 += Neighbor3 * Neighbor3;
            m1 += Neighbor4; m2 += Neighbor4 * Neighbor4;
            m1 += Neighbor5; m2 += Neighbor5 * Neighbor5;
            m1 += Neighbor6; m2 += Neighbor6 * Neighbor6;
            m1 += Neighbor7; m2 += Neighbor7 * Neighbor7;

            float4 mu = m1 * (1.0 / 8.0);
            float4 Sigma = sqrt( abs(m2 * (1.0 / 8.0) - mu * mu) );
            NeighborMin = mu - 1.0 * Sigma;
            NeighborMax = mu + 1.0 * Sigma;
        #else
            float4 NeighborMin = min(min(
                min(min(Neighbor0, Neighbor1), min(Neighbor2, Neighbor3)),
                min(min(Neighbor4, Neighbor5), min(Neighbor6, Neighbor7))), Neighbor8);
            float4 NeighborMax = max(max(
                max(max(Neighbor0, Neighbor1), max(Neighbor2, Neighbor3)),
                max(max(Neighbor4, Neighbor5), max(Neighbor6, Neighbor7))), Neighbor8);
        #endif
    #endif

    // FETCH HISTORY
    // -------------
    #if AA_BICUBIC
        vec2 Weight[3];
        vec2 Sample[3];
        Bicubic2DCatmullRom( BackN.xy, PostprocessInput1Size.xy, Sample, Weight );

        OutColor  = texture( texPrev, vec2( Sample[0].x, Sample[0].y ) ) * Weight[0].x * Weight[0].y;
        OutColor += texture( texPrev, vec2( Sample[1].x, Sample[0].y ) ) * Weight[1].x * Weight[0].y;
        OutColor += texture( texPrev, vec2( Sample[2].x, Sample[0].y ) ) * Weight[2].x * Weight[0].y;

        OutColor += texture( texPrev, vec2( Sample[0].x, Sample[1].y ) ) * Weight[0].x * Weight[1].y;
        OutColor += texture( texPrev, vec2( Sample[1].x, Sample[1].y ) ) * Weight[1].x * Weight[1].y;
        OutColor += texture( texPrev, vec2( Sample[2].x, Sample[1].y ) ) * Weight[2].x * Weight[1].y;

        OutColor += texture( texPrev, vec2( Sample[0].x, Sample[2].y ) ) * Weight[0].x * Weight[2].y;
        OutColor += texture( texPrev, vec2( Sample[1].x, Sample[2].y ) ) * Weight[1].x * Weight[2].y;
        OutColor += texture( texPrev, vec2( Sample[2].x, Sample[2].y ) ) * Weight[2].x * Weight[2].y;
    #else
        OutColor = PostprocessInput1.SampleLevel(PostprocessInput1Sampler, BackN.xy, 0);
    #endif

    #if AA_DYNAMIC_ANTIGHOST && AA_DYNAMIC && !AA_ALPHA && !AA_DOF
        bool Dynamic1 = PostprocessInput3.SampleLevel(PostprocessInput3Sampler, UV, 0, int2( 0, -1)).x > 0;
        bool Dynamic3 = PostprocessInput3.SampleLevel(PostprocessInput3Sampler, UV, 0, int2(-1,  0)).x > 0;
        bool Dynamic4 = PostprocessInput3.SampleLevel(PostprocessInput3Sampler, UV, 0).x > 0;
        bool Dynamic5 = PostprocessInput3.SampleLevel(PostprocessInput3Sampler, UV, 0, int2( 1,  0)).x > 0;
        bool Dynamic7 = PostprocessInput3.SampleLevel(PostprocessInput3Sampler, UV, 0, int2( 0,  1)).x > 0;

        bool Dynamic = DynamicN || Dynamic1 || Dynamic3 || Dynamic4 || Dynamic5 || Dynamic7;
        if( !Dynamic && OutColor.a > 0 )
        {
            OutColor = Filtered;
        }
    #endif

    #if AA_DEBUG
        Neighbor4.rg = float2(0.0, 0.0);
        NeighborMin.rg = float2(0.0, 0.0);
        NeighborMax.rg = float2(0.0, 0.0);
        Filtered.rg = float2(0.0, 0.0);
        FilteredLow.rg = float2(0.0, 0.0);
        float DebugDiffCurrent = Filtered.b;
    #endif
    #if AA_YCOCG
        OutColor.rgb = RGBToYCoCg(OutColor.rgb);
        #if AA_TONE
            OutColor.xyz *= HdrWeightY(OutColor.x, InExposureScale);
        #endif
    #else
        #if AA_GREEN_AS_LUMA
            OutColor.rgb *= HdrWeightG(OutColor.rgb, InExposureScale);
        #else
            OutColor.rgb *= HdrWeight4(OutColor.rgb, InExposureScale);
        #endif
    #endif
    #if AA_DEBUG
        OutColor.rg = float2(0.0, 0.0);
        float DebugDiffPrior = OutColor.b;
    #endif


    // FIND LUMA OF CLAMPED HISTORY
    // ----------------------------
    // Save off luma of history before the clamp.
    #if AA_YCOCG
        float LumaMin = NeighborMin.x;
        float LumaMax = NeighborMax.x;
        float LumaHistory = OutColor.x;
    #else
        #if AA_GREEN_AS_LUMA
            float LumaMin = NeighborMin.g;
            float LumaMax = NeighborMax.g;
            float LumaHistory = OutColor.g;
        #else
            float LumaMin = Luma4(NeighborMin.rgb);
            float LumaMax = Luma4(NeighborMax.rgb);
            float LumaHistory = Luma4(OutColor.rgb);
        #endif
    #endif
    float LumaContrast = LumaMax - LumaMin;
    #if AA_CLAMP
        #if AA_YCOCG
            OutColor.rgb = clamp(OutColor.rgb, NeighborMin.rgb, NeighborMax.rgb);
            #if (AA_ALPHA == 0)
                OutColor.a = clamp(OutColor.a, NeighborMin.a, NeighborMax.a);
            #endif
        #else
            #if AA_AABB
                // Clamp history, this uses color AABB intersection for tighter fit.
                // Clamping works with the low pass (if available) to reduce flicker.
                float ClampBlend = HistoryClamp(OutColor.rgb, FilteredLow.rgb, NeighborMin.rgb, NeighborMax.rgb);
                #if AA_ALPHA
                    OutColor.rgb = lerp(OutColor.rgb, FilteredLow.rgb, ClampBlend);
                #else
                    OutColor.rgba = lerp(OutColor.rgba, FilteredLow.rgba, ClampBlend);
                #endif
            #else
                OutColor = clamp(OutColor, NeighborMin, NeighborMax);
            #endif
        #endif
    #endif
    #if AA_DEBUG
        OutColor.rg = float2(0.0, 0.0);
    #endif

    // ADD BACK IN ALIASING TO SHARPEN
    // -------------------------------
    #if AA_FILTERED && !AA_BICUBIC
        // Blend in non-filtered based on the amount of sub-pixel motion.
        float AddAliasing = saturate(HistoryBlur) * 0.5;
        float LumaContrastFactor = 32.0;
        #if AA_GREEN_AS_LUMA || AA_YCOCG
            // GREEN_AS_LUMA is 1/4 as bright.
            LumaContrastFactor *= 4.0;
        #endif
        AddAliasing = saturate(AddAliasing + rcp(1.0 + LumaContrast * LumaContrastFactor));
        Filtered.rgb = lerp(Filtered.rgb, Neighbor4.rgb, AddAliasing);
    #endif
    #if AA_YCOCG
        float LumaFiltered = Filtered.x;
    #else
        #if AA_GREEN_AS_LUMA
            float LumaFiltered = Filtered.g;
        #else
            float LumaFiltered = Luma4(Filtered.rgb);
        #endif
    #endif

    // COMPUTE BLEND AMOUNT
    // --------------------
    float DistToClamp = min(abs(LumaMin-LumaHistory), abs(LumaMax-LumaHistory));
    #if AA_BICUBIC
        float HistoryFactor = 0.125 * DistToClamp;
    #else
        float HistoryAmount = (1.0/8.0) + HistoryBlur * (1.0/8.0);
        float HistoryFactor = DistToClamp * HistoryAmount * (1.0 + HistoryBlur * HistoryAmount * 8.0);
    #endif
    float BlendFinal = clamp(HistoryFactor / (DistToClamp + LumaMax - LumaMin), 0.0, 1.0);
    #if AA_TONE
        BlendFinal = 0.04;
    #endif
    #if RESPONSIVE
        // Responsive forces 1/4 of new frame.
        BlendFinal = 1.0/4.0;
    #elif AA_NAN && (COMPILER_GLSL || COMPILER_METAL)
        // The current Metal & GLSL compilers don't handle saturate(NaN) -> 0, instead they return NaN/INF.
        BlendFinal = -min(-BlendFinal, 0.0);
    #endif

    // Offscreen feedback resets.
    #if AA_LERP
        float FixedLerp = 1.0/float(AA_LERP);
    #endif
    if(OffScreen)
    {
        OutColor = Filtered;
        #if AA_ALPHA
            OutColor.a = 0.0;
        #endif
        #if AA_LERP
            FixedLerp = 1.0;
        #endif
    }

    // DO FINAL BLEND BETWEEN HISTORY AND FILTERED COLOR
    // -------------------------------------------------
    #if (AA_LERP == 0)
        #if AA_ALPHA
            // Blend in linear to hit luma target.
            OutColor.rgb = lerp(OutColor.rgb, Filtered.rgb, BlendFinal);
            #if RESPONSIVE
                OutColor.a = max(OutColor.a, 1.0/2.0);
            #endif
        #else
            OutColor = mix(OutColor, Filtered, BlendFinal);
            #if AA_FORCE_ALPHA_CLAMP
                OutColor.a = clamp(OutColor.a, NeighborMin.a, NeighborMax.a);
            #endif
        #endif
    #else
        OutColor = lerp(OutColor, Filtered, FixedLerp);
    #endif
    #if AA_YCOCG
        #if AA_TONE
            OutColor.xyz *= HdrWeightInvY(OutColor.x, InExposureScale);
        #endif
        OutColor.rgb = YCoCgToRGB(OutColor.rgb);
    #else
        // Convert back into linear.
        #if AA_GREEN_AS_LUMA
            OutColor.rgb *= HdrWeightInvG(OutColor.rgb, InExposureScale);
        #else
            OutColor.rgb *= HdrWeightInv4(OutColor.rgb, InExposureScale);
        #endif
    #endif
    #if AA_NAN
        // Transform NaNs to black, transform negative colors to black.
        OutColor.rgb = -min(-OutColor.rgb, 0.0);
    #endif
    #if AA_DYNAMIC_ANTIGHOST && AA_DYNAMIC && !AA_ALPHA && !AA_DOF
        OutColor.a = DynamicN ? 1 : 0;
    #endif
    #if AA_DEBUG
        OutColor.g = abs(DebugDiffPrior - DebugDiffCurrent);
        OutColor.r = OutColor.a;
    #endif

    #if 0
        // Test velocity scaling
        OutColor.r *= VelocityScaling.x*.5+.5;
        OutColor.g *= (1 - VelocityScaling.x)*.5+.5;
        OutColor.b *= .5;
    #endif
}
