uniform sampler2D texMain;
uniform float threshold;

in vec2 v_texCoord;

out vec4 OutColor;

float Luminance( vec3 LinearColor )
{
    return dot( LinearColor, vec3( 0.3, 0.59, 0.11 ) );
}

void main()
{
    /*vec4 SceneColor = texture(texMain, v_texCoord);
    SceneColor.rgb = min(vec3(256 * 256, 256 * 256, 256 * 256), SceneColor.rgb);
    vec3 LinearColor = SceneColor.rgb;
    float ExposureScale = 2.0;
//#if NO_EYEADAPTATION_EXPOSURE_FIX
    //ExposureScale = threshold;
//#endif
    // todo: make this adjustable (e.g. LUT)
    float TotalLuminance = Luminance( LinearColor ) * ExposureScale;
    float BloomLuminance = TotalLuminance - threshold;
    // mask 0..1
    float BloomAmount = clamp(BloomLuminance / 2.0f, 0.0, 1.0);
    OutColor = vec4(BloomAmount * LinearColor, 0);*/

    vec3 hdrColor = texture(texMain, v_texCoord).rgb;

    if (dot(hdrColor, vec3(0.2126, 0.7152, 0.0722)) > threshold) {
        OutColor = vec4(hdrColor, 1.0);
    } else {
        OutColor = vec4(0.0, 0.0, 0.0, 1.0);
    }

    //float c = clamp(dot(hdrColor, vec3(0.2126, 0.7152, 0.0722)), 0.0, threshold) / threshold;
    //fragColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(hdrColor, 1.0), smoothstep(0.8, 1.0, c));

    //OutColor = vec4(hdrColor, 1.0) * smoothstep(0.7, 1.0, c);
    //OutColor = vec4(hdrColor, 1.0);
}
