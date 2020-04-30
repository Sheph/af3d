uniform sampler2D texMain;
#ifdef NM
uniform sampler2D texNormal;
#endif
uniform sampler2D texSpecular;

uniform vec4 mainColor;
uniform vec4 specularColor;
uniform float shininess;
uniform vec3 eyePos;
uniform vec4 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform float lightCutoffCos;
uniform float lightCutoffInnerCos;
uniform float lightPower;

in vec2 v_texCoord;
in vec3 v_pos;
#ifdef NM
in mat3 v_tbn;
#else
in vec3 v_normal;
#endif

out vec4 fragColor;

void main()
{
    if (lightPos.w == 0.0) {
        // ambient
        fragColor = texture(texMain, v_texCoord) * mainColor * vec4(lightColor, 1.0);
        return;
    }

#ifdef NM
    vec3 normalDirection = normalize(v_tbn * normalize(2.0 * texture(texNormal, v_texCoord).rgb - 1.0));
#else
    vec3 normalDirection = normalize(v_normal);
#endif
    vec3 viewDirection = normalize(eyePos - v_pos);
    vec3 lightDirection;
    float attenuation;

    if (lightPos.w == 1.0) {
        // directional
        attenuation = 1.0;
        lightDirection = -lightDir;
    } else {
        vec3 positionToLightSource = vec3(lightPos.xyz - v_pos);
        lightDirection = normalize(positionToLightSource);
        attenuation = max(0.0, 1.0 - length(positionToLightSource) / length(lightDir));
        if (lightPos.w == 3.0) {
            // spot
            float spotCosine = dot(-lightDirection, normalize(lightDir));
            if (spotCosine < lightCutoffCos) {
                attenuation = 0.0;
            } else {
                float spotValue = smoothstep(lightCutoffCos, lightCutoffInnerCos, spotCosine);
                attenuation = attenuation * pow(spotValue, lightPower);
            }
        }
    }

    float diffuseCoeff = max(0.0, dot(normalDirection, lightDirection));
    vec4 diffuseReflection = texture(texMain, v_texCoord) * mainColor * vec4(lightColor, 1.0) * diffuseCoeff;

    vec4 specularReflection;
    if (diffuseCoeff <= 0.0) { // light source on the wrong side?
        specularReflection = vec4(0.0); // no specular reflection
    } else { // light source on the right side
        specularReflection = texture(texSpecular, v_texCoord) * specularColor * vec4(lightColor, 1.0) *
            pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), shininess);
    }

    fragColor = attenuation * (diffuseReflection + specularReflection);
}
