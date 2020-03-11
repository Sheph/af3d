#version 330 core

in vec2 v_texCoord;
in vec4 v_color;
uniform sampler2D texMain;

out vec4 fragColor;

void main()
{
    fragColor = texture2D(texMain, v_texCoord) * v_color;
}
