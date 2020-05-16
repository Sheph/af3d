#extension GL_ARB_shading_language_420pack : enable

uniform sampler2D texMain;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = textureLod(texMain, v_texCoord, mipLevel);
}
