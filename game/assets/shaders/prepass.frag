#ifndef SHADOW
in vec4 v_prevClipPos;
in vec4 v_clipPos;

out vec2 fragVelocity;
#endif

void main()
{
#ifdef SHADOW
    // TODO: discard based on alpha value.
#else
    vec2 a = (v_clipPos.xy / v_clipPos.w);
    vec2 b = (v_prevClipPos.xy / v_prevClipPos.w);
    fragVelocity = (a - b);
#endif
}
