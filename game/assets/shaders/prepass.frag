in vec4 v_prevClipPos;
in vec4 v_clipPos;

out vec2 fragVelocity;

void main()
{
    vec2 a = (v_clipPos.xy / v_clipPos.w);
    vec2 b = (v_prevClipPos.xy / v_prevClipPos.w);
    fragVelocity = (a - b);
}
