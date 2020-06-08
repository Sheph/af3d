uniform samplerCube texMain;

in vec3 v_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = texture(texMain, v_texCoord);
}
