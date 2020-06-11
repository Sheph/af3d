uniform sampler2D texMain;
uniform sampler2D texDepth;
uniform float kernel[11];
uniform float kernelOffset[5];
uniform int kernelSize;
uniform int kernelDir;
uniform vec2 argNearFar;

in vec2 v_texCoord;

out float fragColor;

float linearDepth(float d)
{
    d = 2.0 * d - 1.0;
    return 2.0 * argNearFar.x * argNearFar.y / (argNearFar.y + argNearFar.x - d * (argNearFar.y - argNearFar.x));
}

void main()
{
    vec2 texSize = vec2(textureSize(texMain, 0));
    int kSize = (kernelSize - 1) / 2;
    int rSize = (kSize / 2) + 1;

    float centerZ = linearDepth(texture(texDepth, v_texCoord).r);

    float weightSum = kernel[0];
    float tmp = weightSum * texture(texMain, v_texCoord).r;

    if (kernelDir == 0) {
        for (int i = 1; i < rSize; ++i) {
            vec2 c1 = v_texCoord + vec2(0.0, kernelOffset[i]) / texSize;
            vec2 c2 = v_texCoord - vec2(0.0, kernelOffset[i]) / texSize;

            float z1 = linearDepth(texture(texDepth, c1).r);
            float z2 = linearDepth(texture(texDepth, c2).r);
            float diffZ = (centerZ - z1) * 10.0;
            float w1 = kernel[i] / (1.0 + abs(diffZ));
            diffZ = (centerZ - z2) * 10.0;
            float w2 = kernel[i] / (1.0 + abs(diffZ));

            tmp += w1 * texture(texMain, c1).r;
            tmp += w2 * texture(texMain, c2).r;
            weightSum += (w1 + w2);
        }
    } else {
        for (int i = 1; i < rSize; ++i) {
            vec2 c1 = v_texCoord + vec2(kernelOffset[i], 0.0) / texSize;
            vec2 c2 = v_texCoord - vec2(kernelOffset[i], 0.0) / texSize;

            float z1 = linearDepth(texture(texDepth, c1).r);
            float z2 = linearDepth(texture(texDepth, c2).r);
            float diffZ = (centerZ - z1) * 10.0;
            float w1 = kernel[i] / (1.0 + abs(diffZ));
            diffZ = (centerZ - z2) * 10.0;
            float w2 = kernel[i] / (1.0 + abs(diffZ));

            tmp += w1 * texture(texMain, c1).r;
            tmp += w2 * texture(texMain, c2).r;
            weightSum += (w1 + w2);
        }
    }

    fragColor = tmp / weightSum;
}
