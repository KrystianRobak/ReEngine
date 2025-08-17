#pragma once

#include "ReTypes.h"
#include <any>
#include <unordered_map>


class Event
{
public:
	Event() = delete;

	explicit Event(EventType type)
		: mType(type)
	{}

	template<typename T>
	void SetParam(EventType id, T value)
	{
		mData[id] = value;
	}

	template<typename T>
	T GetParam(EventType id)
	{
		return std::any_cast<T>(mData[id]);
	}

	EventType GetType() const
	{
		return mType;
	}

private:
	EventType mType{};
	std::unordered_map<EventType, std::any> mData{};
};
