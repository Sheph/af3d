//#define BLINN
uniform sampler2D texMain;
#ifdef NM
uniform sampler2D texNormal;
#endif
uniform sampler2D texSpecular;
uniform sampler2DArray texShadowCSM;

uniform vec4 mainColor;
uniform vec4 specularColor;
uniform float shininess;
uniform vec3 eyePos;
uniform vec3 ambientColor;
uniform vec4 clusterCfg;
uniform int outputMask;
uniform int immCameraIdx;

struct ClusterLight
{
    vec4 pos;
    vec4 color;
    vec4 dir;
    float cutoffCos;
    float cutoffInnerCos;
    float power;
    uint enabled;
    uint shadowIdx[MAX_IMM_CAMERAS + 1];
};

struct ClusterTileData
{
    uint lightOffset;
    uint lightCount;
    uint probeOffset;
    uint probeCount;
};

struct CSM
{
    float farBounds[4];
    mat4 mat[4];
    uint texIdx[4];
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

layout (std430, binding = 7) readonly buffer shadowCSMSSBO
{
    CSM shadowCSM[];
};

in vec2 v_texCoord;
in vec3 v_pos;
#ifdef NM
in mat3 v_tbn;
#else
in vec3 v_normal;
#endif
in vec4 v_clipPos;

layout (location = 0) out vec4 fragColor;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec4 fragAmbient;

float processCSMShadow(const CSM csm)
{
    int idx = CSM_NUM_SPLITS - 1;
    for (int i = 0; i < CSM_NUM_SPLITS - 1; ++i) {
        if (gl_FragCoord.z < csm.farBounds[i]) {
            idx = i;
            break;
        }
    }

    vec4 shadowPos = vec4(v_pos, 1.0) * csm.mat[idx];

    shadowPos.w = shadowPos.z;
    shadowPos.z = float(csm.texIdx[idx]);

    float shadowD = texture(texShadowCSM, shadowPos.xyz).x;

    // Get the difference of the stored depth and the distance of this fragment to the light.
    float diff = shadowD - shadowPos.w;

    return clamp(diff * 250.0 + 1.0, 0.0, 1.0);
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

    vec4 outColor = vec4(0.0);
    vec4 outAmbient = vec4(0.0);

    vec4 diffuse = texture(texMain, v_texCoord) * mainColor;
    vec4 specular = texture(texSpecular, v_texCoord) * specularColor;

    {
        // ambient
        outAmbient = diffuse * vec4(ambientColor, 1.0);
    }

#ifdef NM
    vec3 normalDirection = normalize(v_tbn * normalize(2.0 * texture(texNormal, v_texCoord).rgb - 1.0));
#else
    vec3 normalDirection = normalize(v_normal);
#endif
    fragNormal = normalDirection;

    vec3 viewDirection = normalize(eyePos - v_pos);
    vec3 lightDirection;
    float attenuation;

    uint lightCount = clusterTileData[tileIndex].lightCount;
    uint lightOffset = clusterTileData[tileIndex].lightOffset;

    for (uint i = 0; i < lightCount; i++) {
        uint lightIndex = clusterLightIndices[lightOffset + i];
        const ClusterLight light = clusterLights[lightIndex];

        if (light.pos.w == 1.0) {
            // directional
            attenuation = 1.0;
            lightDirection = -light.dir.xyz;
            uint csmIdx = light.shadowIdx[immCameraIdx];
            if (csmIdx != -1) {
                attenuation = processCSMShadow(shadowCSM[csmIdx]);
            }
        } else {
            vec3 positionToLightSource = vec3(light.pos.xyz - v_pos);
            lightDirection = normalize(positionToLightSource);
            attenuation = max(0.0, 1.0 - length(positionToLightSource) / length(light.dir.xyz));
            if (light.pos.w == 3.0) {
                // spot
                float spotCosine = dot(-lightDirection, normalize(light.dir.xyz));
                if (spotCosine < light.cutoffCos) {
                    attenuation = 0.0;
                } else {
                    float spotValue = smoothstep(light.cutoffCos, light.cutoffInnerCos, spotCosine);
                    attenuation = attenuation * pow(spotValue, light.power);
                }
            }
        }

        float diffuseCoeff = max(0.0, dot(normalDirection, lightDirection));
        vec4 diffuseReflection = diffuse * vec4(light.color.xyz, 0.0) * diffuseCoeff;

#ifdef BLINN
        vec4 specularReflection = specular * vec4(light.color.xyz, 0.0) *
            pow(max(0.0, dot(normalize(viewDirection + lightDirection), normalDirection)), shininess * 2.0);
#else
        vec4 specularReflection = specular * vec4(light.color.xyz, 0.0) *
            pow(max(0.0, dot(reflect(-lightDirection, normalDirection), viewDirection)), shininess);
#endif

        outColor += attenuation * (diffuseReflection + specularReflection);
    }

    if (outputMask == 7) {
        fragColor = outColor;
        fragAmbient = outAmbient;
    } else {
        fragColor = outColor + outAmbient;
    }
}
