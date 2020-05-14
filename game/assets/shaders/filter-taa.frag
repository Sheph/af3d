uniform sampler2D texMain;
uniform sampler2D texPrev;
uniform sampler2D texNoise;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec2 sz = vec2(textureSize(texMain, 0));

    vec4 texel = texture(texMain, v_texCoord);
    vec4 pixelMovement = texture(texNoise, v_texCoord);
    vec2 oldPixelUv = v_texCoord - ((pixelMovement.xy * 2.0) - 1.0);
    vec4 oldTexel = texture(texPrev, oldPixelUv);

    // Use simple neighbor clamping
    vec4 maxNeighbor = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 minNeighbor = vec4(1.0);
    vec4 average = vec4(0.0);

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 neighborUv = v_texCoord + vec2(float(x) / sz.x, float(y) / sz.y);
            vec4 neighborTexel = texture(texMain, neighborUv);

            maxNeighbor = max(maxNeighbor, neighborTexel);
            minNeighbor = min(minNeighbor, neighborTexel);
            average += neighborTexel / 9.0;
        }
    }

    oldTexel = clamp(oldTexel, minNeighbor, maxNeighbor);

    // UE Method to get rid of flickering. Weight frame mixing amount
    // based on local contrast.
    float contrast = distance(average, texel);
    float weight = 0.05 * contrast;
    vec4 compositeColor = mix(oldTexel, texel, weight);

    fragColor = 1.0 * compositeColor;
}
