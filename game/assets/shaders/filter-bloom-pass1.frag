uniform sampler2D texMain;
uniform float threshold;

in vec2 v_texCoord;

out vec4 fragColor;

float luminance(vec3 linearColor)
{
    return dot(linearColor, vec3(0.3, 0.59, 0.11));
}

void main()
{
    vec4 sceneColor = texture(texMain, v_texCoord);
    sceneColor.rgb = min(vec3(256 * 256, 256 * 256, 256 * 256), sceneColor.rgb);
    vec3 linearColor = sceneColor.rgb;
    float exposureScale = 1.0;
    float totalLuminance = luminance(linearColor) * exposureScale;
    float bloomLuminance = totalLuminance - threshold;
    // mask 0..1
    float bloomAmount = clamp(bloomLuminance / 2.0f, 0.0, 1.0);
    fragColor = vec4(bloomAmount * linearColor, 0);
}
