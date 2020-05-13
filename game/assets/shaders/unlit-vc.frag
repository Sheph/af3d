uniform sampler2D texMain;

in vec2 v_texCoord;
in vec4 v_color;

in vec4 v_prevClipPos;
in vec4 v_clipPos;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    fragColor = texture(texMain, v_texCoord) * v_color;
    OUT_FRAG_VELOCITY();
}
