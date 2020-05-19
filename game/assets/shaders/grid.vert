layout(location = 0) in vec3 pos;
layout(location = 3) in vec4 color;
uniform mat4 viewProj;
uniform mat4 prevStableMVP;
uniform mat4 curStableMVP;

out vec3 v_pos;
out vec4 v_color;

out vec4 v_prevClipPos;
out vec4 v_clipPos;

void main()
{
    v_pos = pos;
    v_color = color;
    gl_Position = vec4(pos, 1.0) * viewProj;

    v_prevClipPos = vec4(pos, 1.0) * prevStableMVP;
    v_clipPos = vec4(pos, 1.0) * curStableMVP;
}
