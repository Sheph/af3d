uniform sampler2D texMain;
uniform float mipLevel;
uniform float kernel[11];
uniform float kernelOffset[5];
uniform int kernelSize;
uniform int kernelDir;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));
    int kSize = (kernelSize - 1) / 2;
    int rSize = (kSize / 2) + 1;
    vec4 tmp = kernel[0] * textureLod(texMain, v_texCoord, mipLevel);
    if (kernelDir == 0) {
        for (int i = 1; i < rSize; ++i) {
            tmp += kernel[i] * textureLod(texMain, v_texCoord + vec2(0.0, kernelOffset[i]) / texSize, mipLevel);
            tmp += kernel[i] * textureLod(texMain, v_texCoord - vec2(0.0, kernelOffset[i]) / texSize, mipLevel);
        }
    } else {
        for (int i = 1; i < rSize; ++i) {
            tmp += kernel[i] * textureLod(texMain, v_texCoord + vec2(kernelOffset[i], 0.0) / texSize, mipLevel);
            tmp += kernel[i] * textureLod(texMain, v_texCoord - vec2(kernelOffset[i], 0.0) / texSize, mipLevel);
        }
    }

    fragColor = tmp;
}
