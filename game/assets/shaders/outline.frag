uniform vec4 mainColor;

in vec4 v_prevClipPos;
in vec4 v_clipPos;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    fragColor = mainColor;
    OUT_FRAG_VELOCITY();
}
