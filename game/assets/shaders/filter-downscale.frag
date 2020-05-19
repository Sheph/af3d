uniform sampler2D texMain;
uniform float mipLevel;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec2 texSize = vec2(textureSize(texMain, int(mipLevel)));

    vec2 offset = 1.0 / texSize;

    vec2 uv[4];
    // blur during downsample (4x4 kernel) to get better quality especially for HDR content.
    uv[0] = v_texCoord + offset * vec2(-1, -1);
    uv[1] = v_texCoord + offset * vec2( 1, -1);
    uv[2] = v_texCoord + offset * vec2(-1,  1);
    uv[3] = v_texCoord + offset * vec2( 1,  1);

    vec4 sample[4];

    for (int i = 0; i < 4; ++i) {
        sample[i] = textureLod(texMain, uv[i], mipLevel);
    }

    fragColor = (sample[0] + sample[1] + sample[2] + sample[3]) * 0.25;

    // fixed rarely occuring yellow color tint of the whole viewport (certain view port size, need to investigate more)
    fragColor.rgb = max(vec3(0,0,0), fragColor.rgb);
}
