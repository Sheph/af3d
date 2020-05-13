layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texCoord;

uniform mat4 viewProj;
uniform vec2 viewportSize;

out vec2 v_texCoord;
out vec2 v_rgbNW;
out vec2 v_rgbNE;
out vec2 v_rgbSW;
out vec2 v_rgbSE;
out vec2 v_rgbM;

void main()
{
    vec2 fragCoord = texCoord * viewportSize;
    vec2 inverseVP = 1.0 / viewportSize.xy;
    v_texCoord = texCoord;
    v_rgbNW = (fragCoord + vec2(-1.0, -1.0)) * inverseVP;
    v_rgbNE = (fragCoord + vec2(1.0, -1.0)) * inverseVP;
    v_rgbSW = (fragCoord + vec2(-1.0, 1.0)) * inverseVP;
    v_rgbSE = (fragCoord + vec2(1.0, 1.0)) * inverseVP;
    v_rgbM = vec2(fragCoord * inverseVP);
    gl_Position = vec4(pos, 1.0) * viewProj;
}
