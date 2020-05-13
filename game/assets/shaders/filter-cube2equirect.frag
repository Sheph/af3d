uniform samplerCube texMain;
uniform float mipLevel;

in vec2 v_texCoord;
in vec4 v_color;

out vec4 fragColor;

void main()
{
    vec2 thetaphi = ((v_texCoord * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);
    vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
    fragColor = textureLod(texMain, rayDirection, mipLevel) * v_color;
}
