#pragma once
#include <vector>
#include <string>
#include <algorithm> 
#include <assimp/anim.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include "AnimHelpers.h"

struct KeyPosition { glm::vec3 position; float timeStamp; };
struct KeyRotation { glm::quat orientation; float timeStamp; };
struct KeyScale { glm::vec3 scale; float timeStamp; };

class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f) {
        m_NumPositions = channel->mNumPositionKeys;
        for (int i = 0; i < m_NumPositions; ++i) {
            KeyPosition data;
            data.position = AssimpToGLM(channel->mPositionKeys[i].mValue);
            data.timeStamp = (float)channel->mPositionKeys[i].mTime;
            m_Positions.push_back(data);
        }
        m_NumRotations = channel->mNumRotationKeys;
        for (int i = 0; i < m_NumRotations; ++i) {
            KeyRotation data;
            data.orientation = AssimpToGLM(channel->mRotationKeys[i].mValue);
            data.timeStamp = (float)channel->mRotationKeys[i].mTime;
            m_Rotations.push_back(data);
        }
        m_NumScalings = channel->mNumScalingKeys;
        for (int i = 0; i < m_NumScalings; ++i) {
            KeyScale data;
            data.scale = AssimpToGLM(channel->mScalingKeys[i].mValue);
            data.timeStamp = (float)channel->mScalingKeys[i].mTime;
            m_Scales.push_back(data);
        }
    }

    Bone(const std::string& name, int ID,
        const std::vector<KeyPosition>& positions,
        const std::vector<KeyRotation>& rotations,
        const std::vector<KeyScale>& scales)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f),
        m_Positions(positions), m_Rotations(rotations), m_Scales(scales) {
        m_NumPositions = (int)positions.size();
        m_NumRotations = (int)rotations.size();
        m_NumScalings = (int)scales.size();
    }

    void Update(float animationTime) {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    glm::mat4 GetLocalTransform() const { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() const { return m_ID; }

private:
    // --- FIXED BINARY SEARCH ---
    template<typename KeyType>
    int GetKeyIndex(const std::vector<KeyType>& keys, float animationTime) {
        if (keys.empty()) return 0;

        // **FIX: Handle edge cases properly**
        if (animationTime <= keys[0].timeStamp) return 0;
        if (animationTime >= keys[keys.size() - 1].timeStamp)
            return std::max(0, (int)keys.size() - 2);

        auto it = std::lower_bound(keys.begin(), keys.end(), animationTime,
            [](const KeyType& key, float time) { return key.timeStamp < time; });

        if (it == keys.begin()) return 0;
        if (it == keys.end()) return (int)keys.size() - 2;

        return std::max(0, (int)std::distance(keys.begin(), it) - 1);
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
        float framesDiff = nextTimeStamp - lastTimeStamp;
        if (framesDiff <= 0.0001f) return 0.0f; // **FIX: Avoid division by zero**
        float scaleFactor = (animationTime - lastTimeStamp) / framesDiff;
        return glm::clamp(scaleFactor, 0.0f, 1.0f); // **FIX: Clamp to valid range**
    }

    glm::mat4 InterpolatePosition(float animationTime) {
        if (m_NumPositions == 0) return glm::mat4(1.0f); // **FIX: Handle empty case**
        if (m_NumPositions == 1) return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0 = GetKeyIndex(m_Positions, animationTime);
        int p1 = std::min(p0 + 1, m_NumPositions - 1); // **FIX: Bounds safety**

        if (p0 == p1) return glm::translate(glm::mat4(1.0f), m_Positions[p0].position);

        float scaleFactor = GetScaleFactor(m_Positions[p0].timeStamp, m_Positions[p1].timeStamp, animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0].position, m_Positions[p1].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    glm::mat4 InterpolateRotation(float animationTime) {
        if (m_NumRotations == 0) return glm::mat4(1.0f); // **FIX: Handle empty case**
        if (m_NumRotations == 1) return glm::toMat4(glm::normalize(m_Rotations[0].orientation));

        int p0 = GetKeyIndex(m_Rotations, animationTime);
        int p1 = std::min(p0 + 1, m_NumRotations - 1); // **FIX: Bounds safety**

        if (p0 == p1) return glm::toMat4(glm::normalize(m_Rotations[p0].orientation));

        float scaleFactor = GetScaleFactor(m_Rotations[p0].timeStamp, m_Rotations[p1].timeStamp, animationTime);
        glm::quat finalRotation = glm::slerp(m_Rotations[p0].orientation, m_Rotations[p1].orientation, scaleFactor);
        return glm::toMat4(glm::normalize(finalRotation));
    }

    glm::mat4 InterpolateScaling(float animationTime) {
        if (m_NumScalings == 0) return glm::mat4(1.0f); // **FIX: Handle empty case**
        if (m_NumScalings == 1) return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0 = GetKeyIndex(m_Scales, animationTime);
        int p1 = std::min(p0 + 1, m_NumScalings - 1); // **FIX: Bounds safety**

        if (p0 == p1) return glm::scale(glm::mat4(1.0f), m_Scales[p0].scale);

        float scaleFactor = GetScaleFactor(m_Scales[p0].timeStamp, m_Scales[p1].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0].scale, m_Scales[p1].scale, scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }

    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions, m_NumRotations, m_NumScalings;
    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;
};