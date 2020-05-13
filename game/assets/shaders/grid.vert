layout(location = 0) in vec3 pos;
layout(location = 3) in vec4 color;
uniform mat4 viewProj;

out vec3 v_pos;
out vec4 v_color;

void main()
{
    v_pos = pos;
    v_color = color;
    gl_Position = vec4(pos, 1.0) * viewProj;
}
