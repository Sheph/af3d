#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 normal;

uniform mat4 viewProj;
uniform mat4 model;

out vec2 v_texCoord;
out vec3 v_normal;
out vec3 v_pos;

void main()
{
    v_texCoord = texCoord;
    v_normal = normalize(normal * mat3(model));
    v_pos = (vec4(pos, 1.0) * model).xyz;

    gl_Position = vec4(pos, 1.0) * model * viewProj;
}
