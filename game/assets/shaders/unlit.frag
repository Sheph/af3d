uniform sampler2D texMain;

uniform vec4 mainColor;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    fragColor = texture(texMain, v_texCoord) * mainColor;
}
