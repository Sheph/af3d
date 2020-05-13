layout(location = 0) in vec3 pos;
layout(location = 3) in vec4 color;
uniform mat4 viewProj;

out vec4 v_color;
out vec3 v_pos;

void main()
{
    v_color = color;
    v_pos = pos;
    gl_Position = vec4(pos, 1.0) * viewProj;
}
