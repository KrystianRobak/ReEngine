#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <cmath>

struct KeyPosition
{
	glm::vec3 position;
	float timeStamp;
};

struct KeyRotation
{
	glm::quat orientation;
	float timeStamp;
};

struct KeyScale
{
	glm::vec3 scale;
	float timeStamp;
};


inline float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
	if (!std::isfinite(lastTimeStamp) || !std::isfinite(nextTimeStamp) || !std::isfinite(animationTime))
		return 0.0f;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	if (framesDiff <= 0.0f)
		return 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float scaleFactor = midWayLength / framesDiff;
	if (scaleFactor < 0.0f) scaleFactor = 0.0f;
	else if (scaleFactor > 1.0f) scaleFactor = 1.0f;
	return scaleFactor;
}

inline glm::mat4 interpolatePosition(float animationTime, KeyPosition from, KeyPosition to)
{
	float scaleFactor = getScaleFactor(from.timeStamp, to.timeStamp, animationTime);
	glm::vec3 finalPosition = glm::mix(from.position, to.position, scaleFactor);
	glm::mat4 translation = glm::translate(glm::mat4(1.0f), finalPosition);
	return translation;
}

inline glm::mat4 interpolateRotation(float animationTime, KeyRotation from, KeyRotation to)
{
	float scaleFactor = getScaleFactor(from.timeStamp, to.timeStamp, animationTime);
	glm::quat fromN = glm::normalize(from.orientation);
	glm::quat toN = glm::normalize(to.orientation);
	if (glm::dot(fromN, toN) < 0.0f) toN = -toN;
	glm::quat finalRotation = glm::slerp(fromN, toN, scaleFactor);
	finalRotation = glm::normalize(finalRotation);
	return glm::toMat4(finalRotation);
}

inline glm::mat4 interpolateScaling(float animationTime, KeyScale from, KeyScale to)
{
	float scaleFactor = getScaleFactor(from.timeStamp, to.timeStamp, animationTime);
	glm::vec3 finalScale = glm::mix(from.scale, to.scale, scaleFactor);
	return glm::scale(glm::mat4(1.0f), finalScale);
}
