uniform vec4 mainColor;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    fragColor = mainColor;
    fragVelocity = vec2(0.0);
}
