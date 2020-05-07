#version 330 core

uniform samplerCube texMain;

in vec4 v_color;
in vec3 v_pos;

out vec4 fragColor;

void main()
{
    fragColor = texture(texMain, v_pos);
}
