#version 330 core

uniform sampler2D texMain;
uniform sampler2D texSpecular;
uniform float mipLevel;
uniform float strength;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec3 hdrScene = texture(texMain, v_texCoord).rgb;
    vec3 hdrBloom = vec3(0.0);
    for (int i = 0; i < int(mipLevel); ++i) {
        hdrBloom += textureLod(texSpecular, v_texCoord, float(i)).rgb;
    }
    fragColor = vec4(hdrScene + hdrBloom * strength, 1.0);
}
