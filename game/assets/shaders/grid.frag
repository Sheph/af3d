uniform vec3 gridPos;
uniform vec3 gridRight;
uniform vec3 gridUp;
uniform float gridStep;
uniform vec3 gridXColor;
uniform vec3 gridYColor;
uniform vec3 eyePos;

in vec3 v_pos;
in vec4 v_color;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

void main()
{
    float step1 = 1.0 / gridStep;
    float step2 = step1 / 10.0;
    vec2 coord = vec2(dot(v_pos - gridPos, gridRight), dot(v_pos - gridPos, gridUp));
    vec2 grid1 = abs(fract(coord * step1 - 0.5) - 0.5) / fwidth(coord * step1 * 1.5f);
    vec2 grid2 = abs(fract(coord * step2 - 0.5) - 0.5) / fwidth(coord * step2 * 2.0f);
    float c1 = 1.0 - min(min(grid1.x, grid1.y), 1.0);
    float c2 = (1.0 - min(min(grid2.x, grid2.y), 1.0)) * 1.5;
    float c2X = (1.0 - min(grid2.x, 1.0)) * 1.5;
    float c2Y = (1.0 - min(grid2.y, 1.0)) * 1.5;
    float factor = 1.0 - abs(dot(normalize(eyePos - v_pos), cross(gridRight, gridUp)));
    factor = 1.0 - factor * factor * factor;
    if ((abs(coord.x) < gridStep / 2) && (c2X > 0.0)) {
        fragColor = vec4(gridYColor, c2X * factor * v_color.a);
    } else if ((abs(coord.y) < gridStep / 2) && (c2Y > 0.0)) {
        fragColor = vec4(gridXColor, c2Y * factor * v_color.a);
    } else {
        fragColor = vec4(1.0, 1.0, 1.0, max(c1, c2) * factor) * v_color;
    }
    fragVelocity = vec2(0.0);
}
