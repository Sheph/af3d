uniform sampler2D texMain;
#ifdef NM
uniform sampler2D texNormal;
#endif
#ifdef FAST
uniform sampler2D texSpecular;
#else
uniform sampler2D texRoughness;
uniform sampler2D texMetalness;
uniform sampler2D texAO;
#endif
uniform sampler2D texEmissive;
uniform samplerCubeArray texIrradiance;
uniform samplerCubeArray texSpecularCM;
uniform sampler2D texSpecularLUT;

uniform vec4 mainColor;
uniform vec3 eyePos;
uniform float emissiveFactor;
#ifdef NM
uniform int normalFormat;
#endif
uniform vec4 clusterCfg;

struct ClusterLight
{
    vec4 pos;
    vec4 color;
    vec4 dir;
    float cutoffCos;
    float cutoffInnerCos;
    float power;
    uint enabled;
};

struct ClusterProbe
{
    vec4 pos;
    mat4 invModel;
    uint cubeIdx;
    uint spherical;
    uint enabled;
    float padding;
};

struct ClusterTileData
{
    uint lightOffset;
    uint lightCount;
    uint probeOffset;
    uint probeCount;
};

layout (std430, binding = 2) readonly buffer clusterTileDataSSBO
{
    ClusterTileData clusterTileData[];
};

layout (std430, binding = 3) readonly buffer clusterLightIndicesSSBO
{
    uint clusterLightIndices[];
};

layout (std430, binding = 4) readonly buffer clusterLightsSSBO
{
    ClusterLight clusterLights[];
};

layout (std430, binding = 5) readonly buffer clusterProbeIndicesSSBO
{
    uint clusterProbeIndices[];
};

layout (std430, binding = 6) readonly buffer clusterProbesSSBO
{
    ClusterProbe clusterProbes[];
};

in vec2 v_texCoord;
in vec3 v_pos;
#ifdef NM
in mat3 v_tbn;
#else
in vec3 v_normal;
#endif

in vec4 v_clipPos;

out vec4 fragColor;

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
    return alphaSq / (PI * denom * denom + Epsilon);
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
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta + Epsilon, 5.0);
}

const float blendPower = 12.0;

// See: https://seblagarde.wordpress.com/2012/09/29/image-based-lighting-approaches-and-parallax-corrected-cubemap/
vec3 reflDirectionFixupBox(mat4 lightProbeInvModel, vec3 lightProbePos, vec3 ReflDirectionWS, vec3 PositionWS, out float blendFactor)
{
    // Intersection with OBB convertto unit box space
    // Transform in local unit parallax cube space (scaled and rotated)
    vec3 RayLS = ReflDirectionWS * mat3(lightProbeInvModel);
    vec3 PositionLS = (vec4(PositionWS, 1.0) * lightProbeInvModel).xyz;

    vec3 Unitary = vec3(1.0, 1.0, 1.0);
    vec3 FirstPlaneIntersect = (Unitary - PositionLS) / RayLS;
    vec3 SecondPlaneIntersect = (-Unitary - PositionLS) / RayLS;
    vec3 FurthestPlane = max(FirstPlaneIntersect, SecondPlaneIntersect);
    float Distance = min(FurthestPlane.x, min(FurthestPlane.y, FurthestPlane.z));

    // Use Distance in WS directly to recover intersection
    vec3 IntersectPositionWS = PositionWS + ReflDirectionWS * Distance;
    ReflDirectionWS = IntersectPositionWS - lightProbePos;

    blendFactor = 1.0 - pow(clamp(max(abs(PositionLS.x), max(abs(PositionLS.y), abs(PositionLS.z))), 0.0, 1.0), blendPower);

    return ReflDirectionWS;
}

vec3 reflDirectionFixupSphere(mat4 lightProbeInvModel, vec3 lightProbePos, vec3 ReflDirectionWS, vec3 PositionWS, out float blendFactor)
{
    vec3 PositionLS = (vec4(PositionWS, 1.0) * lightProbeInvModel).xyz;
    vec3 LightProbePositionLS = (vec4(lightProbePos, 1.0) * lightProbeInvModel).xyz;

    ReflDirectionWS = ReflDirectionWS + (PositionLS - LightProbePositionLS);

    blendFactor = 1.0 - pow(clamp(length(PositionLS), 0.0, 1.0), blendPower);

    return ReflDirectionWS;
}

float linearDepth(float depthRange)
{
    float linear = 2.0 * clusterCfg.x * clusterCfg.y / (clusterCfg.y + clusterCfg.x - depthRange * (clusterCfg.y - clusterCfg.x));
    return linear;
}

void main()
{
    vec4 ndc = v_clipPos / v_clipPos.w;
    uint zTile = uint(max(log2(linearDepth(ndc.z)) * clusterCfg.z + clusterCfg.w, 0.0));
    uvec3 tiles = uvec3(uvec2((0.5 * (ndc.xy + 1.0)) * vec2(CLUSTER_GRID_X, CLUSTER_GRID_Y)), zTile);
    uint tileIndex = tiles.x + CLUSTER_GRID_X * tiles.y + (CLUSTER_GRID_X * CLUSTER_GRID_Y) * tiles.z;

    vec4 albedoFull = texture(texMain, v_texCoord) * mainColor;
    vec3 albedo = albedoFull.rgb;
#ifdef FAST
    vec3 fast = texture(texSpecular, v_texCoord).rgb;
    float ao = 1.0;
    float roughness = fast.g;
    float metalness = fast.b;
#else
    float metalness = texture(texMetalness, v_texCoord).r;
    float roughness = texture(texRoughness, v_texCoord).r;
#endif

    vec3 Lo = normalize(eyePos - v_pos);

#ifdef NM
    vec3 tN = 2.0 * texture(texNormal, v_texCoord).rgb - 1;
    if (normalFormat == 1) {
        tN.b = sqrt(1.0 - tN.r * tN.r - tN.g * tN.g);
    }
    vec3 N = normalize(v_tbn * tN);
#else
    vec3 N = normalize(v_normal);
#endif

    // Angle between surface normal and outgoing light direction.
    float cosLo = max(0.0, dot(N, Lo));

    // Fresnel reflectance at normal incidence (for metals use albedo color).
    vec3 F0 = mix(Fdielectric, albedo, metalness);

    uint probeCount = clusterTileData[tileIndex].probeCount;
    uint probeOffset = clusterTileData[tileIndex].probeOffset;

    {
        // ambient

#ifndef FAST
        float ao = texture(texAO, v_texCoord).r;
#endif
        vec3 emissive = texture(texEmissive, v_texCoord).rgb * emissiveFactor;

        // Specular reflection vector.
        const vec3 Lr = 2.0 * cosLo * N - Lo;

        float totalBlend = 0.0;
        vec3 totalIrradiance = vec3(0.0);
        vec3 totalSpecularIrradiance = vec3(0.0);

        for (uint i = 1; i <= probeCount; i++) {
            uint probeIndex = clusterProbeIndices[probeOffset + i];
            const ClusterProbe probe = clusterProbes[probeIndex];
            float layer = float(probe.cubeIdx);

            float blendFactor;
            vec3 LrFixed;
            if (probe.spherical == 1) {
                LrFixed = reflDirectionFixupSphere(probe.invModel, probe.pos.xyz, normalize(Lr), v_pos, blendFactor);
            } else {
                LrFixed = reflDirectionFixupBox(probe.invModel, probe.pos.xyz, normalize(Lr), v_pos, blendFactor);
            }

            // Sample diffuse irradiance at normal direction.
            vec3 irradiance = texture(texIrradiance, vec4(N, layer)).rgb;

            // Sample pre-filtered specular reflection environment at correct mipmap level.
            vec3 specularIrradiance = textureLod(texSpecularCM, vec4(LrFixed, layer), roughness * SPECULAR_CM_LEVELS).rgb;

            totalIrradiance += (1.0 - totalBlend) * blendFactor * irradiance;
            totalSpecularIrradiance += (1.0 - totalBlend) * blendFactor * specularIrradiance;
            totalBlend = blendFactor + (1.0 - blendFactor) * totalBlend;

            if (totalBlend >= 1.0) {
                break;
            }
        }

        if (totalBlend < 0.99) {
            totalIrradiance = mix(texture(texIrradiance, vec4(N, 0.0)).rgb, totalIrradiance, totalBlend);
            totalSpecularIrradiance = mix(textureLod(texSpecularCM, vec4(Lr, 0.0), roughness * SPECULAR_CM_LEVELS).rgb, totalSpecularIrradiance, totalBlend);
        }

        // Calculate Fresnel term for ambient lighting.
        // Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
        // use cosLo instead of angle with light's half-vector (cosLh above).
        // See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
        vec3 F = fresnelSchlick(F0, cosLo);

        // Get diffuse contribution factor (as with direct lighting).
        vec3 kd = mix(vec3(1.0) - F, vec3(0.0), metalness);

        // Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
        vec3 diffuseIBL = kd * albedo * totalIrradiance;

        // Split-sum approximation factors for Cook-Torrance specular BRDF.
        vec2 specularBRDF = texture(texSpecularLUT, vec2(cosLo, roughness)).rg;

        // Total specular IBL contribution.
        vec3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * totalSpecularIrradiance;

        // Total ambient lighting contribution.
        fragColor = vec4((diffuseIBL + specularIBL) * ao + emissive, albedoFull.a);
    }

    uint lightCount = clusterTileData[tileIndex].lightCount;
    uint lightOffset = clusterTileData[tileIndex].lightOffset;
    vec3 Li;
    float attenuation;

    for (uint i = 0; i < lightCount; i++) {
        uint lightIndex = clusterLightIndices[lightOffset + i];
        const ClusterLight light = clusterLights[lightIndex];

        if (light.pos.w == 1.0) {
            // directional
            attenuation = 1.0;
            Li = -light.dir.xyz;
        } else {
            vec3 positionToLightSource = vec3(light.pos.xyz - v_pos);
            Li = normalize(positionToLightSource);
            attenuation = max(0.0, 1.0 - length(positionToLightSource) / length(light.dir.xyz));
            if (attenuation <= 0.0) {
                continue;
            }
            if (light.pos.w == 3.0) {
                // spot
                float spotCosine = dot(-Li, normalize(light.dir.xyz));
                if (spotCosine < light.cutoffCos) {
                    attenuation = 0.0;
                    continue;
                } else {
                    float spotValue = smoothstep(light.cutoffCos, light.cutoffInnerCos, spotCosine);
                    attenuation = attenuation * pow(spotValue, light.power);
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
        fragColor += vec4((diffuseBRDF + specularBRDF) * light.color.xyz * cosLi * attenuation, 0.0);
    }
}
