#pragma once
#include <cstdint>
#include <atomic>
#include "ReTypes.h"

struct PendingDeletion {
    Entity entity;
    uint64_t deletionEpoch;
};

class EpochManager
{
public:
    uint64_t GetGlobalEpoch() const
    {
        return globalEpoch.load();
    }
    uint64_t GetRenderEpoch() const
    {
        return renderEpoch.load();
    }
    uint64_t GetPhysicsEpoch() const
    {
        return physicsEpoch.load();
    }
    uint64_t GetGameEpoch() const
    {
        return gameEpoch.load();
    }
    void IncrementGlobalEpoch()
    {
        globalEpoch.fetch_add(1);
    }
    void IncrementRenderEpoch()
    {
        renderEpoch.fetch_add(1);
    }
    void IncrementPhysicsEpoch()
    {
        physicsEpoch.fetch_add(1);
    }
    void IncrementGameEpoch()
    {
        gameEpoch.fetch_add(1);
	}
private:
    std::atomic<uint64_t> globalEpoch = 0;
    std::atomic<uint64_t> renderEpoch = 0;
    std::atomic<uint64_t> physicsEpoch = 0;
    std::atomic<uint64_t> gameEpoch = 0;

};

