uniform sampler2D texMain;
uniform float mipLevel;
uniform float gaussianKernel[11];
uniform float gaussianOffset[5];
uniform int gaussianMSize;
uniform int gaussianDir;

in vec2 v_texCoord;
in vec2 v_a1[5];
in vec2 v_a2[5];

out vec4 fragColor;

void main()
{
    float weight[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));
    int kSize = (gaussianMSize - 1) / 2;
    int rSize = (kSize / 2) + 1;
    vec4 tmp = gaussianKernel[0] * textureLod(texMain, v_texCoord, mipLevel);
    /*if (gaussianDir == 0) {
        for (int i = 1; i < rSize; ++i) {
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord + vec2(0.0, gaussianOffset[i]) / texSize, mipLevel);
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord - vec2(0.0, gaussianOffset[i]) / texSize, mipLevel);
        }
    } else {
        for (int i = 1; i < rSize; ++i) {
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord + vec2(gaussianOffset[i], 0.0) / texSize, mipLevel);
            tmp += gaussianKernel[i] * textureLod(texMain, v_texCoord - vec2(gaussianOffset[i], 0.0) / texSize, mipLevel);
        }
    }*/

    for (int i = 1; i < rSize; ++i) {
        tmp += gaussianKernel[i] * textureLod(texMain, v_a1[i], mipLevel);
        tmp += gaussianKernel[i] * textureLod(texMain, v_a2[i], mipLevel);
    }

     /*vec2 tex_offset = 1.0 / textureSize(texMain, 0); // gets size of single texel
     vec3 result = textureLod(texMain, v_texCoord, mipLevel).rgb * weight[0];
     if(gaussianDir != 0)
     {
         for(int i = 1; i < 5; ++i)
         {
            result += textureLod(texMain, v_texCoord + vec2(tex_offset.x * i, 0.0), mipLevel).rgb * weight[i];
            result += textureLod(texMain, v_texCoord - vec2(tex_offset.x * i, 0.0), mipLevel).rgb * weight[i];
         }
     }
     else
     {
         for(int i = 1; i < 5; ++i)
         {
             result += textureLod(texMain, v_texCoord + vec2(0.0, tex_offset.y * i), mipLevel).rgb * weight[i];
             result += textureLod(texMain, v_texCoord - vec2(0.0, tex_offset.y * i), mipLevel).rgb * weight[i];
         }
     }
     fragColor = vec4(result, 1.0);*/

    fragColor = tmp;
}
