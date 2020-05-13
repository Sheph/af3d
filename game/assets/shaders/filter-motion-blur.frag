#define MAX_SAMPLES 5

uniform sampler2D texMain;
uniform sampler2D texNoise;
uniform float realDt;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(texMain, 0));
    vec2 velocity = texture(texNoise, v_texCoord).rg * (2.0 / (realDt * 60.0));

    float speed = length(velocity / texelSize);
    int nSamples = clamp(int(speed), 1, MAX_SAMPLES);

    vec3 res = texture(texMain, v_texCoord).rgb;
    for (int i = 1; i < nSamples; ++i) {
        vec2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
        res += texture(texMain, v_texCoord + offset).rgb;
    }
    res /= float(nSamples);

    fragColor = vec4(res, 1.0);
}
