layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;
layout(location = 3) in vec4 color;
uniform mat4 modelViewProj;
uniform mat4 oldMVP;

out vec2 v_texCoord;
out vec4 v_color;

out vec4 v_prevClipPos;
out vec4 v_clipPos;

void main()
{
    v_texCoord = texCoord;
    v_color = color;

    v_prevClipPos = vec4(pos, 1.0) * oldMVP;
    v_clipPos = vec4(pos, 1.0) * modelViewProj;

    gl_Position = v_clipPos;
}
