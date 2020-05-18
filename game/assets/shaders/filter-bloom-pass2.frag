#extension GL_ARB_shading_language_420pack : enable

uniform sampler2D texMain;
uniform sampler2D texSpecular;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    float weights[5] = { 0.5, 0.5, 0.5, 0.5, 0.5 };

    if (mipLevel >= 0.0) {
        fragColor = textureLod(texMain, v_texCoord, mipLevel);
    } else {
        vec3 c = vec3(0.0);
        for (int i = 0; i < 5; ++i) {
            c += textureLod(texSpecular, v_texCoord, float(i)).rgb * weights[i];
        }
        vec3 hdrScene = texture(texMain, v_texCoord).rgb;
        fragColor = vec4(hdrScene + c / 5.0, 1.0);
    }
}
