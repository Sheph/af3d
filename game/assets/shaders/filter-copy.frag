#extension GL_ARB_shading_language_420pack : enable

uniform sampler2D texMain;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 OutColor;

void main()
{
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));

    vec2 Offset = 1 / texSize;

    vec2 UV[4];

    // blur during downsample (4x4 kernel) to get better quality especially for HDR content, can be made an option of this shader
    UV[0] = v_texCoord + Offset * vec2(-1, -1);
    UV[1] = v_texCoord + Offset * vec2( 1, -1);
    UV[2] = v_texCoord + Offset * vec2(-1,  1);
    UV[3] = v_texCoord + Offset * vec2( 1,  1);

    vec4 Sample[4];

    for(int i = 0; i < 4; ++i)
    {
        Sample[i] = textureLod(texMain, UV[i], mipLevel);
    }

    OutColor = (Sample[0] + Sample[1] + Sample[2] + Sample[3]) * 0.25;

    // fixed rarely occuring yellow color tint of the whole viewport (certain view port size, need to investigate more)
    OutColor.rgb = max(vec3(0,0,0), OutColor.rgb);
}
