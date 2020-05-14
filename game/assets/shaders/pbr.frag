uniform sampler2D texMain;
#ifdef NM
uniform sampler2D texNormal;
#endif
uniform sampler2D texRoughness;
uniform sampler2D texMetalness;
uniform samplerCube texIrradiance;
uniform samplerCube texSpecularCM;
uniform sampler2D texSpecularLUT;

uniform vec4 mainColor;
uniform vec3 eyePos;
uniform vec4 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDir;
uniform float lightCutoffCos;
uniform float lightCutoffInnerCos;
uniform float lightPower;
uniform int specularCMLevels;

in vec2 v_texCoord;
in vec3 v_pos;
#ifdef NM
in mat3 v_tbn;
#else
in vec3 v_normal;
#endif

in vec2 screenSpaceVel;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec2 fragVelocity;

const float PI = 3.141592;
const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    vec3 albedo = texture(texMain, v_texCoord).rgb * mainColor.rgb;
    float metalness = texture(texMetalness, v_texCoord).r;
    float roughness = texture(texRoughness, v_texCoord).r;

    vec3 Lo = normalize(eyePos - v_pos);

#ifdef NM
    vec3 N = normalize(v_tbn * normalize(2.0 * texture(texNormal, v_texCoord).rgb - 1.0));
#else
    vec3 N = normalize(v_normal);
#endif

    // Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(N, Lo));

    // Specular reflection vector.
    vec3 Lr = 2.0 * cosLo * N - Lo;

    // Fresnel reflectance at normal incidence (for metals use albedo color).
    vec3 F0 = mix(Fdielectric, albedo, metalness);

    if (lightPos.w == 0.0) {
        // ambient

        // Sample diffuse irradiance at normal direction.
        vec3 irradiance = texture(texIrradiance, N).rgb;

        // Calculate Fresnel term for ambient lighting.
        // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
        // use cosLo instead of angle with light's half-vector (cosLh above).
        // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
        vec3 F = fresnelSchlick(F0, cosLo);

        // Get diffuse contribution factor (as with direct lighting).
        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

        // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
        vec3 diffuseIBL = kd * albedo * irradiance;

        // Sample pre-filtered specular reflection environment at correct mipmap level.
        vec3 specularIrradiance = textureLod(texSpecularCM, Lr, roughness * specularCMLevels).rgb;

        // Split-sum approximation factors for Cook-Torrance specular BRDF.
        vec2 specularBRDF = texture(texSpecularLUT, vec2(cosLo, roughness)).rg;

        // Total specular IBL contribution.
        vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;

        // Total ambient lighting contribution.
        fragColor = vec4(diffuseIBL + specularIBL, 1.0);
        fragVelocity = screenSpaceVel;
        return;
    }

    vec3 Li;
    float attenuation;

    if (lightPos.w == 1.0) {
        // directional
        attenuation = 1.0;
        Li = -lightDir;
    } else {
        vec3 positionToLightSource = vec3(lightPos.xyz - v_pos);
        Li = normalize(positionToLightSource);
        attenuation = max(0.0, 1.0 - length(positionToLightSource) / length(lightDir));
        if (lightPos.w == 3.0) {
            // spot
            float spotCosine = dot(-Li, normalize(lightDir));
            if (spotCosine < lightCutoffCos) {
                attenuation = 0.0;
            } else {
                float spotValue = smoothstep(lightCutoffCos, lightCutoffInnerCos, spotCosine);
                attenuation = attenuation * pow(spotValue, lightPower);
            }
        }
    }

    // Half-vector between Li and Lo.
    vec3 Lh = normalize(Li + Lo);

    // Calculate angles between surface normal and various light vectors.
    float cosLi = max(0.0, dot(N, Li));
    float cosLh = max(0.0, dot(N, Lh));

    // Calculate Fresnel term for direct lighting.
    vec3 F  = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
    // Calculate normal distribution for specular BRDF.
    float D = ndfGGX(cosLh, roughness);
    // Calculate geometric attenuation for specular BRDF.
    float G = gaSchlickGGX(cosLi, cosLo, roughness);

    // Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
    // Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
    // To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
    vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

    // Lambert diffuse BRDF.
    // We don't scale by 1/PI for lighting & material units to be more convenient.
    // See: https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
    vec3 diffuseBRDF = kd * albedo;

    // Cook-Torrance specular microfacet BRDF.
    vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * cosLo);

    // Total contribution for this light.
    fragColor = vec4((diffuseBRDF + specularBRDF) * lightColor * cosLi * attenuation, 1.0);
}
