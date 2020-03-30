#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 2) in vec3 normal;

uniform mat4 modelViewProj;
uniform vec2 viewportSize;

void main()
{
    vec4 clipPos = vec4(pos, 1.0) * modelViewProj;
    vec3 clipNormal = normal * mat3(modelViewProj);

    // 1.5 - outline width in pixels
    clipPos.xy += normalize(clipNormal.xy) / viewportSize * 1.5 * clipPos.w * 2.0;

    gl_Position = clipPos;
}
