uniform sampler2D texMain;
uniform float threshold;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
    vec3 hdrColor = texture(texMain, v_texCoord).rgb;
    //if (dot(hdrColor, vec3(0.2126, 0.7152, 0.0722)) > threshold) {
      //  fragColor = vec4(hdrColor, 1.0);
    //} else {
      //  fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    //}

    float c = clamp(dot(hdrColor, vec3(0.2126, 0.7152, 0.0722)), 0.0, threshold) / threshold;
    //fragColor = mix(vec4(0.0, 0.0, 0.0, 1.0), vec4(hdrColor, 1.0), smoothstep(0.8, 1.0, c));

    fragColor = vec4(hdrColor, 1.0) * smoothstep(0.9, 1.0, c);
}
