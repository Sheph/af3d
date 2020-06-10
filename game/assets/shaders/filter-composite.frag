uniform sampler2D texMain;
uniform sampler2D texSpecular;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(texMain, v_texCoord).rgb + texture(texSpecular, v_texCoord).rgb, 1.0);
}
