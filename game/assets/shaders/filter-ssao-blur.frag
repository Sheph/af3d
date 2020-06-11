uniform sampler2D texMain;

in vec2 v_texCoord;

out float fragColor;

void main()
{
    const int blurRange = 2;
    int n = 0;
    vec2 texelSize = 1.0 / vec2(textureSize(texMain, 0));
    float result = 0.0;
    for (int x = -blurRange; x < blurRange; x++)
    {
        for (int y = -blurRange; y < blurRange; y++)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(texMain, v_texCoord + offset).r;
            n++;
        }
    }

    fragColor = result / (float(n));
}
