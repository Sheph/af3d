layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
uniform mat4 modelViewProj;

out vec2 v_texCoord;

void main()
{
    v_texCoord = texCoord;
    gl_Position = vec4(pos, 1.0) * modelViewProj;
}
