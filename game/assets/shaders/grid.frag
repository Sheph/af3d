#version 330 core

uniform vec3 gridPos;
uniform vec3 gridRight;
uniform vec3 gridUp;
uniform float gridStep;
uniform vec3 eyePos;

in vec3 v_pos;
in vec4 v_color;

out vec4 fragColor;

void main()
{
    float step1 = 1.0 / gridStep;
    float step2 = step1 / 10.0;
    vec2 coord = vec2(dot(v_pos - gridPos, gridRight), dot(v_pos - gridPos, gridUp));
    vec2 grid1 = abs(fract(coord * step1 - 0.5) - 0.5) / fwidth(coord * step1 * 1.5f);
    vec2 grid2 = abs(fract(coord * step2 - 0.5) - 0.5) / fwidth(coord * step2 * 2.0f);
    vec4 c1 = vec4(1.0, 1.0, 1.0, 1.0 - min(min(grid1.x, grid1.y), 1.0));
    vec4 c2 = vec4(1.0, 1.0, 1.0, (1.0 - min(min(grid2.x, grid2.y), 1.0)) * 1.5);
    float factor = 1.0 - abs(dot(normalize(eyePos - v_pos), cross(gridRight, gridUp)));
    factor = 1.0 - factor * factor * factor;
    fragColor = max(c1, c2) * v_color * factor;
}
