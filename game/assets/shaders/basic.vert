#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

uniform mat4 proj;
uniform vec3 eyePos;
uniform vec4 lightPos;

out vec2 v_texCoord;
out vec3 v_viewDir;
out float v_lightType;
out vec3 v_lightDir;

void main()
{
    v_texCoord = texCoord;
    v_viewDir = pos - eyePos;
    v_lightType = lightPos.w;
    v_lightDir = pos - lightPos.xyz;

    gl_Position = vec4(pos, 1.0) * proj;
}
