uniform sampler2D texMain;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    const float gamma = 2.2;
    const float exposure = 1.0;
    const float pureWhite = 1.0;

    vec3 color = texture(texMain, v_texCoord).rgb * exposure;

    // Reinhard tonemapping operator.
    // see: "Photographic Tone Reproduction for Digital Images", eq. 4
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance/(pureWhite*pureWhite))) / (1.0 + luminance);

    // Scale color by ratio of average luminances.
    vec3 mappedColor = (mappedLuminance / luminance) * color;

    // Gamma correction.
    fragColor = vec4(pow(mappedColor, vec3(1.0/gamma)), 1.0);
}
