#version 330 core

uniform sampler2D texMain;
uniform sampler2D texSpecular;

uniform vec4 mainColor;
uniform vec4 specularColor;
uniform float shininess;
uniform vec3 eyePos;
uniform vec4 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDir;

in vec2 v_texCoord;
in vec3 v_normal;
in vec3 v_pos;

out vec4 fragColor;

void main()
{
    if (lightPos.w == 0.0) {
        // ambient
        fragColor = texture(texMain, v_texCoord) * mainColor * vec4(lightColor, 1.0);
        return;
    }

    vec3 normalDirection = normalize(v_normal);
    vec3 viewDirection = normalize(eyePos - v_pos);
    vec3 lightDirection;
    float attenuation;

    if (lightPos.w == 1.0) {
        // directional
        attenuation = 1.0;
        lightDirection = -lightDir;
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
