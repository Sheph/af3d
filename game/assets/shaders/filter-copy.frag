#extension GL_ARB_shading_language_420pack : enable

uniform sampler2D texMain;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 fragColor;

vec2 offsets[9] = {
    vec2( 1, 1), vec2( 0, 1), vec2(-1, 1),
    vec2( 1, 0), vec2( 0, 0), vec2(-1, 0),
    vec2( 1,-1), vec2( 0,-1), vec2(-1,-1)
    };

void main()
{
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));

    //fragColor = textureLod(texMain, v_texCoord, mipLevel);

     vec4 color = vec4(0,0,0,0);
    for(int i=0;i<9;i++)
        color += textureLod(texMain, v_texCoord + (2.0 / texSize * offsets[i]), mipLevel);
    fragColor = color / 9.0f;
}
