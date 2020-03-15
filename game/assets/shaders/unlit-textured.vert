#version 330 core

layout(location = 0) in vec4 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in vec4 color;
uniform mat4 proj;

out vec2 v_texCoord;
out vec4 v_color;

void main()
{
    v_texCoord = texCoord;
    v_color = color;
    gl_Position = pos * proj;
}
