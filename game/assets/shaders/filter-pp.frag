#version 330 core

uniform sampler2D texMain;
uniform vec2 viewportSize;

in vec2 v_texCoord;
in vec2 v_rgbNW;
in vec2 v_rgbNE;
in vec2 v_rgbSW;
in vec2 v_rgbSE;
in vec2 v_rgbM;

out vec4 fragColor;

vec4 toneMap(vec2 texCoord)
{
    const float gamma = 2.2;
    const float exposure = 1.0;
    const float pureWhite = 1.0;

    vec3 color = texture(texMain, texCoord).rgb * exposure;

    // Reinhard tonemapping operator.
    // see: "Photographic Tone Reproduction for Digital Images", eq. 4
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

    // Scale color by ratio of average luminances.
    vec3 mappedColor = (mappedLuminance / luminance) * color;

    // Gamma correction.
    return vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);
}

#define FXAA_REDUCE_MIN (1.0/ 128.0)
#define FXAA_REDUCE_MUL (1.0 / 8.0)
#define FXAA_SPAN_MAX 8.0

// optimized version for mobile, where dependent
// texture reads can be a bottleneck
vec4 fxaa(vec2 fragCoord, vec2 resolution)
{
    vec4 color;
    mediump vec2 inverseVP = vec2(1.0 / resolution.x, 1.0 / resolution.y);
    vec3 rgbNW = toneMap(v_rgbNW).xyz;
    vec3 rgbNE = toneMap(v_rgbNE).xyz;
    vec3 rgbSW = toneMap(v_rgbSW).xyz;
    vec3 rgbSE = toneMap(v_rgbSE).xyz;
    vec4 texColor = toneMap(v_rgbM);
    vec3 rgbM  = texColor.xyz;
    vec3 luma = vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);
    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    mediump vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
        (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
        max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
        dir * rcpDirMin)) * inverseVP;

    vec3 rgbA = 0.5 * (
        toneMap(fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5)).xyz +
        toneMap(fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        toneMap(fragCoord * inverseVP + dir * -0.5).xyz +
        toneMap(fragCoord * inverseVP + dir * 0.5).xyz);

    float lumaB = dot(rgbB, luma);
    if ((lumaB < lumaMin) || (lumaB > lumaMax)) {
        color = vec4(rgbA, texColor.a);
    } else {
        color = vec4(rgbB, texColor.a);
    }

    return color;
}

void main()
{
    fragColor = fxaa(v_texCoord * viewportSize, viewportSize);
}
