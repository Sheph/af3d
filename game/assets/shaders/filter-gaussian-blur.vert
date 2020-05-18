layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in vec4 color;
uniform mat4 viewProj;
uniform int gaussianDir;
uniform int gaussianMSize;
uniform float mipLevel;
uniform sampler2D texMain;
uniform float gaussianOffset[5];

out vec2 v_texCoord;
out vec4 v_color;
out vec2 v_a1[5];
out vec2 v_a2[5];

void main()
{
    int kSize = (gaussianMSize - 1) / 2;
    int rSize = (kSize / 2) + 1;
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));

    v_texCoord = texCoord;
    v_color = color;
    gl_Position = vec4(pos, 1.0) * viewProj;

    if (gaussianDir == 0) {
        for (int i = 1; i < rSize; ++i) {
            v_a1[i] = v_texCoord + vec2(0, gaussianOffset[i] / texSize.y);
            v_a2[i] = v_texCoord - vec2(0, gaussianOffset[i] / texSize.y);
        }
    } else {
        for (int i = 1; i < rSize; ++i) {
            v_a1[i] = v_texCoord + vec2(gaussianOffset[i] / texSize.x, 0);
            v_a2[i] = v_texCoord - vec2(gaussianOffset[i] / texSize.x, 0);
        }
    }
}
