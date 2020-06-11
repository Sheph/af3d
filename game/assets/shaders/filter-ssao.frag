uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texNoise;

uniform vec2 viewportSize;
uniform vec3 kernel[64];
uniform int kernelSize;
uniform float radius;
uniform mat4 argViewProj;
uniform vec2 argNearFar;

mat4 invArgViewProj;

in vec2 v_texCoord;

out float fragColor;

float linearDepth(float d)
{
    d = 2.0 * d - 1.0;
    return 2.0 * argNearFar.x * argNearFar.y / (argNearFar.y + argNearFar.x - d * (argNearFar.y - argNearFar.x));
}

void main()
{
    invArgViewProj = inverse(argViewProj);

    // Early out if there is no data in the normal buffer at this particular sample
    vec3 normal = texture(texNormal, v_texCoord).xyz;
    if (normal == vec3(0.0)) {
        fragColor = 1.0;
        return;
    }

    float fragDepth = texture(texDepth, v_texCoord).r;
    float fragPosZ = linearDepth(fragDepth);
    vec4 fragPos = vec4(2.0 * v_texCoord - 1.0, 2.0 * fragDepth - 1.0, 1.0) * invArgViewProj;
    fragPos /= fragPos.w;

    vec3 randomVec = texture(texNoise, v_texCoord * (viewportSize / vec2(textureSize(texNoise, 0)))).xyz;

    // Make a TBN matrix to go from tangent -> world space (so we can put our hemipshere tangent sample points into world space)
    // Since our normal is already in world space, this TBN matrix will take our tangent space vector and put it into world space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Calculate the total amount of occlusion for this fragment
    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; ++i) {
        vec3 sampleWorld = fragPos.xyz + (TBN * kernel[i]) * radius;

        // Take our sample position in world space and convert it to screen coordinates
        vec4 sampleScreenSpace = vec4(sampleWorld, 1.0);
        sampleScreenSpace = sampleScreenSpace * argViewProj; // World -> View -> Clip Space
        sampleScreenSpace.xyz /= sampleScreenSpace.w; // Perspective division (Clip Space -> NDC)
        sampleScreenSpace.xyz = (sampleScreenSpace.xyz * 0.5) + 0.5; // [-1, 1] -> [0, 1]

        // get sample depth
        float sampleDepth = linearDepth(texture(texDepth, sampleScreenSpace.xy).r);
        float sampleZ = linearDepth(sampleScreenSpace.z);

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPosZ - sampleDepth));
        occlusion += (sampleZ >= sampleDepth + 0.025 ? 1.0 : 0.0) * rangeCheck;
    }

    // Finally we need to normalize our occlusion factor
    occlusion = 1.0 - (occlusion / kernelSize);

    fragColor = pow(occlusion, 3.0);
}
