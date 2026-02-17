
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/scene.h>

#include <vector>

#include "Interpolation.h"

class Bone
{
private:
	glm::mat4 transform;
	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	std::vector<KeyScale> scales;
	size_t numPositions;
	size_t numRotations;
	size_t numScalings;
	std::string name;
	unsigned int id;
	float lastUpdateTime_ = -1.0f;
public:

	Bone(const std::string& name, int ID,
		const std::vector<KeyPosition>& pos,
		const std::vector<KeyRotation>& rot,
		const std::vector<KeyScale>& scl)
		: name(name), id(ID), transform(1.0f)
	{
		positions = pos;
		rotations = rot;
		scales = scl;
		numPositions = positions.size();
		numRotations = rotations.size();
		numScalings = scales.size();
	}

	Bone(const std::string& inName, int inId, const aiNodeAnim* channel) {
		name = inName;
		id = inId;
		transform = glm::mat4(1.0f);

		numPositions = channel->mNumPositionKeys;

		const size_t POS_STRIDE = 32;
		const char* rawPosBase = reinterpret_cast<const char*>(channel->mPositionKeys);

		for (int i = 0; i < numPositions; ++i)
		{
			const char* keyPtr = rawPosBase + (i * POS_STRIDE);

			double timeStamp = *reinterpret_cast<const double*>(keyPtr);

			const float* vecPtr = reinterpret_cast<const float*>(keyPtr + 4);

			KeyPosition data;
			data.timeStamp = (float)timeStamp;

			data.position = glm::vec3((float)vecPtr[0], (float)vecPtr[1], (float)vecPtr[2]);

			positions.push_back(data);
		}

		numRotations = channel->mNumRotationKeys;
		const size_t ROT_STRIDE = 32;
		const char* rawRotBase = reinterpret_cast<const char*>(channel->mRotationKeys);

		for (int i = 0; i < numRotations; ++i)
		{
			const char* keyPtr = rawRotBase + (i * ROT_STRIDE);
			double timeStamp = *reinterpret_cast<const double*>(keyPtr);

			float* valPtr = (float*)(keyPtr + 8);

			KeyRotation data;
			data.timeStamp = (float)timeStamp;
			data.orientation = glm::quat(valPtr[0], valPtr[1], valPtr[2], valPtr[3]);
			rotations.push_back(data);
		}

		numScalings = channel->mNumScalingKeys;
		const size_t SCL_STRIDE = 32;
		const char* rawSclBase = reinterpret_cast<const char*>(channel->mScalingKeys);

		for (int i = 0; i < numScalings; ++i)
		{
			const char* keyPtr = rawSclBase + (i * SCL_STRIDE);

			double timeStamp = *reinterpret_cast<const double*>(keyPtr);
			const float* vecPtr = reinterpret_cast<const float*>(keyPtr + 4);

			KeyScale data;
			data.timeStamp = (float)timeStamp;
			data.scale = glm::vec3((float)vecPtr[0], (float)vecPtr[1], (float)vecPtr[2]);

			scales.push_back(data);
		}
	}

	KeyPosition getPositions(float animationTime) {
		size_t posIndex = (animationTime == 0.0f) ? 0 : getPositionIndex(animationTime) + 1;
		return positions[posIndex];
	}

	KeyRotation getRotations(float animationTime) {
		size_t rotIndex = (animationTime == 0.0f) ? 0 : getRotationIndex(animationTime) + 1;
		return rotations[rotIndex];
	}

	KeyScale getScalings(float animationTime) {
		size_t sclIndex = (animationTime == 0.0f) ? 0 : getScaleIndex(animationTime) + 1;
		return scales[sclIndex];
	}


	void update(float animationTime)
	{
		if (std::abs(animationTime - lastUpdateTime_) < 0.0001f) {
			return;
		}
		lastUpdateTime_ = animationTime;

		size_t posIndex = getPositionIndex(animationTime);
		glm::mat4 translation;
		if (numPositions == 1) {
			translation = glm::translate(glm::mat4(1.0f), positions[0].position);
		}
		else
			translation = interpolatePosition(animationTime, positions[posIndex], positions[posIndex + 1]);

		size_t rotIndex = getRotationIndex(animationTime);
		glm::mat4 rotation;
		if (numRotations == 1)
			rotation = glm::toMat4(glm::normalize(rotations[0].orientation));
		else
			rotation = interpolateRotation(animationTime, rotations[rotIndex], rotations[rotIndex + 1]);

		size_t sclIndex = getScaleIndex(animationTime);
		glm::mat4 scale;
		if (numScalings == 1)
			scale = glm::scale(glm::mat4(1.0f), scales[0].scale);
		else
			scale = interpolateScaling(animationTime, scales[sclIndex], scales[sclIndex + 1]);

		transform = translation * rotation * scale;
	}

	glm::mat4 getTransform() { return transform; }
	std::string getBoneName() const { return name; }
	void SetID(int newID) { id = newID; }
	unsigned int getId() const { return id; }

	size_t getPositionIndex(float animationTime)
	{
		for (size_t index = 0; index < numPositions - 1; ++index)
		{
			if (animationTime < positions[index + 1].timeStamp)
				return index;
		}
		return numPositions - 2;
	}

	size_t getRotationIndex(float animationTime)
	{
		for (size_t index = 0; index < numRotations - 1; ++index)
		{
			if (animationTime < rotations[index + 1].timeStamp)
				return index;
		}
		return numRotations - 2;
	}

	size_t getScaleIndex(float animationTime)
	{
		for (size_t index = 0; index < numScalings - 1; ++index)
		{
			if (animationTime < scales[index + 1].timeStamp)
				return index;
		}
		return numScalings - 2;
	}
};


