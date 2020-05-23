uniform samplerCube texMain;

in vec3 v_texCoord;

in vec4 v_prevClipPos;
in vec4 v_clipPos;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    fragColor = texture(texMain, v_texCoord);
    OUT_FRAG_VELOCITY();
}
