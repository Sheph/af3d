#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 3) in vec4 color;
uniform mat4 proj;

out vec4 v_color;

void main()
{
    v_color = color;
    gl_Position = pos * proj;
}
