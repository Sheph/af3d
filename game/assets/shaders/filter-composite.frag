uniform sampler2D texMain;
uniform sampler2D texSpecular;
uniform sampler2D texAO;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(texMain, v_texCoord).rgb * texture(texAO, v_texCoord).r + texture(texSpecular, v_texCoord).rgb, 1.0);
    //fragColor = vec4(vec3(texture(texAO, v_texCoord).r), 1.0);
}
