uniform sampler2D texMain;
uniform sampler2D texSpecular;
uniform float mipLevel;
uniform float strength;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec3 c = vec3(0.0);
    for (int i = 0; i < int(mipLevel); ++i) {
        c += textureLod(texSpecular, v_texCoord, float(i)).rgb * strength;
    }
    fragColor = vec4(texture(texMain, v_texCoord).rgb + c, 1.0);
}
