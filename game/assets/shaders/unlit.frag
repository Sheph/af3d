#version 330 core

uniform sampler2D texMain;

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

void main()
{
    fragColor = texture(texMain, v_texCoord) * v_color;
}
