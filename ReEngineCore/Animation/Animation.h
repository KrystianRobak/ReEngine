#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "Bone.h"
#include "AssimpNodeData.h"
#include "AnimHelpers.h"

class Animation
{
public:
    Animation() = default;

    Animation(const aiAnimation* animation, const aiScene* scene)
    {
        m_Duration = (float)animation->mDuration;
        m_TicksPerSecond = (animation->mTicksPerSecond != 0) ? (float)animation->mTicksPerSecond : 25.0f;
        m_Name = animation->mName.C_Str();

        ReadMissingBones(animation, scene);
        ReadHeirarchyData(m_RootNode, scene->mRootNode);
    }

    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& bone)
            {
                return bone.GetBoneName() == name;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
    inline float GetDuration() const { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    inline std::string GetName() const { return m_Name; }
    void SetRootNode(const AssimpNodeData& node) { m_RootNode = node; }

	void SetDuration(float duration) { m_Duration = duration; }
	void SetTicksPerSecond(int ticksPerSecond) { m_TicksPerSecond = ticksPerSecond; }

    void AddBone(const Bone& bone)
    {
        m_Bones.push_back(bone);
	}

private:
    void ReadMissingBones(const aiAnimation* animation, const aiScene* scene)
    {
        int size = animation->mNumChannels;

        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            // ID is not strictly needed here as we map by name during update
            m_Bones.push_back(Bone(channel->mNodeName.data, -1, channel));
        }
    }

    void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        if (!src) return;

        dest.name = src->mName.data;
        dest.transformation = AssimpToGLM(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    float m_Duration;
    int m_TicksPerSecond;
    std::vector<Bone> m_Bones;
    AssimpNodeData m_RootNode;
    std::string m_Name;
};