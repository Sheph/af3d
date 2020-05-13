uniform sampler2D texMain;

uniform vec4 mainColor;

in vec2 v_texCoord;

in vec4 v_prevClipPos;
in vec4 v_clipPos;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    fragColor = texture(texMain, v_texCoord) * mainColor;
    OUT_FRAG_VELOCITY();
}
