#extension GL_ARB_shading_language_420pack : enable

uniform sampler2D texMain;
uniform sampler2D texSpecular;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    //float weights[5] = { 0.08, 0.25, 0.75, 1.5, 2.5 };
    float weights[5] = { 0.8, 0.8, 0.8, 0.8, 0.8 };
    float tot = 0.0;
    for (int i = 1; i < 5; ++i) {
        tot += weights[i];
    }
    for (int i = 1; i < 5; ++i) {
        //weights[i] /= tot;
    }

    if (mipLevel == 4.0) {
        fragColor = textureLod(texMain, v_texCoord, 4.0);
    } else if (mipLevel >= 0.0) {
        fragColor = textureLod(texMain, v_texCoord, mipLevel) + textureLod(texSpecular, v_texCoord, mipLevel + 1.0) * weights[int(mipLevel) + 1];
    } else {
        vec3 hdrScene = texture(texMain, v_texCoord).rgb;
        fragColor = vec4(hdrScene + textureLod(texSpecular, v_texCoord, 0.0).rgb * weights[0], 1.0);
    }
}
