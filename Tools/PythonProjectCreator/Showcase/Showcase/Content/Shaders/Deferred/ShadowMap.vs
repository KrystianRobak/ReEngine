#version 460 core
layout (location = 0) in vec3 aPos;
// Note: We don't need normals/UVs for shadow depth, just position and bones
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;
layout (location = 10) in mat4 aInstanceMatrix;

uniform mat4 lightSpaceMatrix;

const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBones[MAX_BONES];
uniform bool uIsAnimated;

void main()
{
    mat4 totalModelMatrix;

    if (uIsAnimated) 
    {
        mat4 BoneTransform = mat4(0.0f);
        float totalWeight = 0.0f;
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(aBoneIDs[i] == -1 || aBoneIDs[i] >= MAX_BONES) 
                continue;
            
            BoneTransform += finalBones[aBoneIDs[i]] * aWeights[i];
            totalWeight += aWeights[i];
        }
        if (totalWeight == 0.0f) BoneTransform = mat4(1.0f);

        totalModelMatrix = aInstanceMatrix * BoneTransform;
    }
    else
    {
        totalModelMatrix = aInstanceMatrix;
    }

    gl_Position = lightSpaceMatrix * totalModelMatrix * vec4(aPos, 1.0);
}