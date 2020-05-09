#version 330 core

uniform sampler2D texMain;

in vec4 v_color;
in vec3 v_pos;

out vec4 fragColor;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(v_pos));
    vec3 color = texture(texMain, uv).rgb;
    fragColor = vec4(color, 1.0);
}
