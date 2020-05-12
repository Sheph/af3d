#version 330 core

uniform sampler2D texMain;
uniform float mipLevel;
uniform float gaussianKernel[11];
uniform float gaussianOffset[5];
uniform int gaussianMSize;
uniform int gaussianDir;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));
    int kSize = (gaussianMSize - 1) / 2;
    int rSize = (kSize / 2) + 1;
    vec4 tmp = gaussianKernel[0] * textureLod(texMain, v_texCoord, mipLevel);
    if (gaussianDir == 0) {
        for (int i = 1; i < rSize; ++i) {
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord + vec2(0.0, gaussianOffset[i]) / texSize, mipLevel);
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord - vec2(0.0, gaussianOffset[i]) / texSize, mipLevel);
        }
    } else {
        for (int i = 1; i < rSize; ++i) {
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord + vec2(gaussianOffset[i], 0.0) / texSize, mipLevel);
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord - vec2(gaussianOffset[i], 0.0) / texSize, mipLevel);
        }
    }
    fragColor = tmp;
}
