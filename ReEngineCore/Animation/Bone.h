#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/scene.h>

#include <vector>
#include <algorithm>
#include <cmath>

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

	static inline bool IsFinite(float v) { return std::isfinite(v); }
	static inline bool IsFiniteVec3(const glm::vec3& v) { return IsFinite(v.x) && IsFinite(v.y) && IsFinite(v.z); }
	static inline bool IsFiniteQuat(const glm::quat& q) { return IsFinite(q.w) && IsFinite(q.x) && IsFinite(q.y) && IsFinite(q.z); }

	static inline glm::quat NormalizeSafe(const glm::quat& q)
	{
		if (!IsFiniteQuat(q)) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		float len2 = glm::dot(q, q);
		if (len2 <= 0.0f || !IsFinite(len2)) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		return glm::normalize(q);
	}

	template<typename T>
	static inline void SortKeysByTime(std::vector<T>& keys)
	{
		std::sort(keys.begin(), keys.end(), [](const T& a, const T& b) {
			return a.timeStamp < b.timeStamp;
			});
	}

	void SanitizeKeys()
	{
		for (auto& p : positions) {
			if (!IsFinite(p.timeStamp) || !IsFiniteVec3(p.position)) {
				p.timeStamp = 0.0f;
				p.position = glm::vec3(0.0f);
			}
		}
		for (auto& r : rotations) {
			if (!IsFinite(r.timeStamp) || !IsFiniteQuat(r.orientation)) {
				r.timeStamp = 0.0f;
				r.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			}
			else {
				r.orientation = NormalizeSafe(r.orientation);
			}
		}
		for (auto& s : scales) {
			if (!IsFinite(s.timeStamp) || !IsFiniteVec3(s.scale)) {
				s.timeStamp = 0.0f;
				s.scale = glm::vec3(1.0f);
			}
		}
		SortKeysByTime(positions);
		SortKeysByTime(rotations);
		SortKeysByTime(scales);

		numPositions = positions.size();
		numRotations = rotations.size();
		numScalings = scales.size();
	}

	KeyPosition SamplePosition(float animationTime) const
	{
		KeyPosition out;
		out.timeStamp = animationTime;
		if (!IsFinite(animationTime)) {
			out.position = glm::vec3(0.0f);
			return out;
		}
		if (numPositions == 0) {
			out.position = glm::vec3(0.0f);
			return out;
		}
		if (numPositions == 1) return positions[0];
		if (animationTime <= positions.front().timeStamp) return positions.front();
		if (animationTime >= positions.back().timeStamp) return positions.back();
		size_t index = getPositionIndex(animationTime);
		float scaleFactor = getScaleFactor(positions[index].timeStamp, positions[index + 1].timeStamp, animationTime);
		out.position = glm::mix(positions[index].position, positions[index + 1].position, scaleFactor);
		return out;
	}

	KeyRotation SampleRotation(float animationTime) const
	{
		KeyRotation out;
		out.timeStamp = animationTime;
		if (!IsFinite(animationTime)) {
			out.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			return out;
		}
		if (numRotations == 0) {
			out.orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			return out;
		}
		if (numRotations == 1) return rotations[0];
		if (animationTime <= rotations.front().timeStamp) return rotations.front();
		if (animationTime >= rotations.back().timeStamp) return rotations.back();
		size_t index = getRotationIndex(animationTime);
		float scaleFactor = getScaleFactor(rotations[index].timeStamp, rotations[index + 1].timeStamp, animationTime);
		glm::quat fromN = NormalizeSafe(rotations[index].orientation);
		glm::quat toN = NormalizeSafe(rotations[index + 1].orientation);
		if (glm::dot(fromN, toN) < 0.0f) toN = -toN;
		out.orientation = glm::normalize(glm::slerp(fromN, toN, scaleFactor));
		return out;
	}

	KeyScale SampleScale(float animationTime) const
	{
		KeyScale out;
		out.timeStamp = animationTime;
		if (!IsFinite(animationTime)) {
			out.scale = glm::vec3(1.0f);
			return out;
		}
		if (numScalings == 0) {
			out.scale = glm::vec3(1.0f);
			return out;
		}
		if (numScalings == 1) return scales[0];
		if (animationTime <= scales.front().timeStamp) return scales.front();
		if (animationTime >= scales.back().timeStamp) return scales.back();
		size_t index = getScaleIndex(animationTime);
		float scaleFactor = getScaleFactor(scales[index].timeStamp, scales[index + 1].timeStamp, animationTime);
		out.scale = glm::mix(scales[index].scale, scales[index + 1].scale, scaleFactor);
		return out;
	}
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
		SanitizeKeys();
	}

	Bone(const std::string& inName, int inId, const aiNodeAnim* channel) {
		name = inName;
		id = inId;
		transform = glm::mat4(1.0f);

		numPositions = channel->mNumPositionKeys;

		for (int i = 0; i < numPositions; ++i)
		{
			KeyPosition data;
			data.timeStamp = (float)channel->mPositionKeys[i].mTime;
			const aiVector3D& v = channel->mPositionKeys[i].mValue;
			data.position = glm::vec3(v.x, v.y, v.z);

			positions.push_back(data);
		}

		numRotations = channel->mNumRotationKeys;

		for (int i = 0; i < numRotations; ++i)
		{
			KeyRotation data;
			data.timeStamp = (float)channel->mRotationKeys[i].mTime;
			const aiQuaternion& q = channel->mRotationKeys[i].mValue;
			data.orientation = glm::quat(q.w, q.x, q.y, q.z);
			rotations.push_back(data);
		}

		numScalings = channel->mNumScalingKeys;

		for (int i = 0; i < numScalings; ++i)
		{
			KeyScale data;
			data.timeStamp = (float)channel->mScalingKeys[i].mTime;
			const aiVector3D& v = channel->mScalingKeys[i].mValue;
			data.scale = glm::vec3(v.x, v.y, v.z);

			scales.push_back(data);
		}

		SanitizeKeys();
	}

	KeyPosition getPositions(float animationTime) {
		return SamplePosition(animationTime);
	}

	KeyRotation getRotations(float animationTime) {
		return SampleRotation(animationTime);
	}

	KeyScale getScalings(float animationTime) {
		return SampleScale(animationTime);
	}


	void update(float animationTime)
	{
		if (std::abs(animationTime - lastUpdateTime_) < 0.0001f) {
			return;
		}
		lastUpdateTime_ = animationTime;

		size_t posIndex = getPositionIndex(animationTime);
		glm::mat4 translation;
		if (numPositions == 0) translation = glm::mat4(1.0f);
		else if (numPositions == 1) translation = glm::translate(glm::mat4(1.0f), positions[0].position);
		else translation = interpolatePosition(animationTime, positions[posIndex], positions[posIndex + 1]);

		size_t rotIndex = getRotationIndex(animationTime);
		glm::mat4 rotation;
		if (numRotations == 0) rotation = glm::mat4(1.0f);
		else if (numRotations == 1)
			rotation = glm::toMat4(NormalizeSafe(rotations[0].orientation));
		else
			rotation = interpolateRotation(animationTime, rotations[rotIndex], rotations[rotIndex + 1]);

		size_t sclIndex = getScaleIndex(animationTime);
		glm::mat4 scale;
		if (numScalings == 0) scale = glm::mat4(1.0f);
		else if (numScalings == 1)
			scale = glm::scale(glm::mat4(1.0f), scales[0].scale);
		else
			scale = interpolateScaling(animationTime, scales[sclIndex], scales[sclIndex + 1]);

		transform = translation * rotation * scale;
	}

	glm::mat4 getTransform() { return transform; }
	std::string getBoneName() const { return name; }
	void SetID(int newID) { id = newID; }
	unsigned int getId() const { return id; }

	size_t getPositionIndex(float animationTime) const
	{
		if (numPositions < 2) return 0;
		for (size_t index = 0; index < numPositions - 1; ++index)
		{
			if (animationTime < positions[index + 1].timeStamp)
				return index;
		}
		return numPositions - 2;
	}

	size_t getRotationIndex(float animationTime) const
	{
		if (numRotations < 2) return 0;
		for (size_t index = 0; index < numRotations - 1; ++index)
		{
			if (animationTime < rotations[index + 1].timeStamp)
				return index;
		}
		return numRotations - 2;
	}

	size_t getScaleIndex(float animationTime) const
	{
		if (numScalings < 2) return 0;
		for (size_t index = 0; index < numScalings - 1; ++index)
		{
			if (animationTime < scales[index + 1].timeStamp)
				return index;
		}
		return numScalings - 2;
	}
};
