layout(location = 0) in vec3 pos;

uniform mat4 modelViewProj;

out vec3 v_texCoord;

void main()
{
    v_texCoord = pos;
    vec4 p = vec4(pos, 1.0) * modelViewProj;
    gl_Position = p.xyww;
}
