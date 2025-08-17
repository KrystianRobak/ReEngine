#pragma once

#include "ReTypes.h"
#include "Event.h"
#include <functional>
#include <list>
#include <unordered_map>

class EventManager
{
public:
	void AddListener(EventType eventType, std::function<void(Event&)> const& listener)
	{
		listeners[eventType].push_back(listener);
	}

	void SendEvent(Event& event)
	{
		std::string type = event.GetType();

		for (auto const& listener : listeners[type])
		{
			listener(event);
		}
	}

	void SendEvent(EventType eventType)
	{
		Event event(eventType);

		for (auto const& listener : listeners[eventType])
		{
			listener(event);
		}
	}

private:
	std::unordered_map<EventType, std::list<std::function<void(Event&)>>> listeners;
};
