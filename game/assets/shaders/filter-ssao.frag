uniform sampler2D texDepth;
uniform sampler2D texNormal;
uniform sampler2D texNoise;

uniform vec2 viewportSize;
uniform vec3 kernel[64];
uniform int kernelSize;
uniform float radius;
uniform mat4 argViewProj;

mat4 invArgViewProj;

in vec2 v_texCoord;

out float fragColor;

vec3 WorldPosFromDepth(vec2 textureCoordinates)
{
    vec4 WSP = vec4(2.0 * textureCoordinates - 1.0, 2.0 * texture(texDepth, textureCoordinates).r - 1.0, 1.0) * invArgViewProj;
    WSP /= WSP.w;
    return WSP.xyz;
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

    vec3 fragPos = WorldPosFromDepth(v_texCoord);
    vec3 randomVec = texture(texNoise, v_texCoord * (viewportSize / vec2(textureSize(texNoise, 0)))).xyz;

    // Make a TBN matrix to go from tangent -> world space (so we can put our hemipshere tangent sample points into world space)
    // Since our normal is already in world space, this TBN matrix will take our tangent space vector and put it into world space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // Calculate the total amount of occlusion for this fragment
    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; ++i) {
        vec3 sampleWorld = fragPos + (TBN * kernel[i]) * radius;

        // Take our sample position in world space and convert it to screen coordinates
        vec4 sampleScreenSpace = vec4(sampleWorld, 1.0);
        sampleScreenSpace = sampleScreenSpace * argViewProj; // World -> View -> Clip Space
        sampleScreenSpace.xyz /= sampleScreenSpace.w; // Perspective division (Clip Space -> NDC)
        sampleScreenSpace.xyz = (sampleScreenSpace.xyz * 0.5) + 0.5; // [-1, 1] -> [0, 1]

        // Check if our current samples depth is behind the screen space geometry's depth, if so then we know it is occluded in screenspace
        float sceneDepth = texture(texDepth, sampleScreenSpace.xy).r;

        // Peform a range check on the current fragment we are calculating the occlusion factor for, and the occlusion position
        vec3 occlusionPos = WorldPosFromDepth(sampleScreenSpace.xy);
        vec3 fragToOcclusionPos = fragPos - occlusionPos;
        if (dot(fragToOcclusionPos, fragToOcclusionPos) <= radius * radius) {
            occlusion += ((sampleScreenSpace.z > sceneDepth) ? 1.0 : 0.0);
        }
    }

    // Finally we need to normalize our occlusion factor
    occlusion = 1.0 - (occlusion / kernelSize);

    fragColor = pow(occlusion, 3.0);
}
