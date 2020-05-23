layout(location = 0) in vec3 pos;

uniform mat4 modelViewProj;
uniform mat4 prevStableMVP;
uniform mat4 curStableMVP;

out vec3 v_texCoord;

out vec4 v_prevClipPos;
out vec4 v_clipPos;

void main()
{
    v_prevClipPos = vec4(pos, 1.0) * prevStableMVP;
    v_clipPos = vec4(pos, 1.0) * curStableMVP;

    v_texCoord = pos;
    vec4 p = vec4(pos, 1.0) * modelViewProj;
    gl_Position = p.xyww;
}
