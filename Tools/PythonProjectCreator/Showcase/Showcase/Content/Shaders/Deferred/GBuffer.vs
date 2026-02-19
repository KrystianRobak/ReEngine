#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

// Bone Data (Locations must match generateAttribute in AssetManager)
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;

// Instance Matrix
layout (location = 10) in mat4 aInstanceMatrix;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out mat3 TBN;

uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBones[MAX_BONES];
uniform bool uIsAnimated;

void main()
{
    mat4 totalModelMatrix;

    if (uIsAnimated) 
    {
        mat4 BoneTransform = mat4(0.0);
        float totalWeight = 0.0;

        for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
        {
            int id = aBoneIDs[i];
            float w = aWeights[i];

            if (id < 0 || id >= MAX_BONES || w <= 0.0)
        continue;

            BoneTransform += finalBones[id] * w;
            totalWeight += w;
        }

        if (totalWeight > 0.0)
            BoneTransform /= totalWeight;
        else
            BoneTransform = mat4(1.0);


        // Safety: If no valid weights, use identity to prevent mesh disappearing
        if (totalWeight == 0.0f) BoneTransform = mat4(1.0f);
        
        // Combine: Instance * Bone
        totalModelMatrix = aInstanceMatrix * BoneTransform;
    }
    else
    {
        totalModelMatrix = aInstanceMatrix;
    }

    vec4 worldPos = totalModelMatrix * vec4(aPos, 1.0);
    FragPos = worldPos.xyz; 
    TexCoords = aTexCoords;

    // Transform Normal to World Space (Handles scaling/rotation correctly)
    mat3 normalMatrix = transpose(inverse(mat3(totalModelMatrix)));
    Normal = normalMatrix * aNormal;

    // Calculate TBN for Normal Mapping
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    TBN = mat3(T, B, N);
    
    gl_Position = projection * view * worldPos;
}