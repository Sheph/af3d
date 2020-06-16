layout(location = 0) in vec3 pos;

uniform mat4 viewProj;
#ifndef SHADOW
uniform mat4 prevStableMVP;
uniform mat4 curStableMVP;

out vec4 v_prevClipPos;
out vec4 v_clipPos;
#endif

void main()
{
#ifndef SHADOW
    v_prevClipPos = vec4(pos, 1.0) * prevStableMVP;
    v_clipPos = vec4(pos, 1.0) * curStableMVP;
#endif
    gl_Position = vec4(pos, 1.0) * viewProj;
}
