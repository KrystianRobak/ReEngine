// GenericComponentArray.h (Dirty Sync Version)

#include "ReTypes.h"
#include <vector>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <cstring>
#include <algorithm>

using BufferPtr = std::vector<char>*;

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
    virtual void* GetData(Entity entity) = 0;
    virtual void SwapData() = 0;
};

class GenericComponentArray : public IComponentArray
{
public:
    GenericComponentArray(size_t componentSize, bool IsDoubleBuffered)
        : mComponentSize(componentSize),
        mIsDoubleBuffered(IsDoubleBuffered)
    {
        if (mIsDoubleBuffered)
        {
            readBufferPtr = &mComponentData;
            writeBufferPtr = &mComponentDataSecond;
        }
        else
        {
            readBufferPtr = &mComponentData;
            writeBufferPtr = &mComponentData;
        }
    }

    void InsertData(Entity entity, void* componentData)
    {
        assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() &&
            "Component added to same entity more than once.");

        size_t newIndex = mSize;
        mEntityToIndexMap[entity] = newIndex;
        mIndexToEntityMap[newIndex] = entity;

        auto currentWriteBuffer = writeBufferPtr.load();

        // Ensure both buffers are big enough
        if ((newIndex + 1) * mComponentSize > currentWriteBuffer->size()) {
            mComponentData.resize((newIndex + 20) * mComponentSize);
            if (mIsDoubleBuffered) {
                mComponentDataSecond.resize((newIndex + 1) * mComponentSize);
            }
        }

        // 1. Write to WRITE buffer (Standard Update)
        memcpy(&((*currentWriteBuffer)[newIndex * mComponentSize]), componentData, mComponentSize);

        // 2. [FIX] Write to READ buffer immediately
        // This prevents the "Frame 1 Glitch" where the renderer sees (0,0,0) before the first sync.
        if (mIsDoubleBuffered)
        {
            auto currentReadBuffer = readBufferPtr.load();
            memcpy(&((*currentReadBuffer)[newIndex * mComponentSize]), componentData, mComponentSize);
        }

        // Mark as dirty (still needed to track future updates)
        mDirtyEntities.insert(entity);

        ++mSize;
    }

    void UpdateData(Entity entity, void* componentData)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
            "Updating non-existent component.");

        size_t index = mEntityToIndexMap[entity];
        auto currentWriteBuffer = writeBufferPtr.load();

        memcpy(&((*currentWriteBuffer)[index * mComponentSize]), componentData, mComponentSize);

        // Mark as dirty
        mDirtyEntities.insert(entity);
    }

    void RemoveData(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
            "Removing non-existent component.");

        size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
        size_t indexOfLastElement = mSize - 1;

        auto currentWriteBuffer = writeBufferPtr.load();

        // Move last element into removed slot
        void* dest = &((*currentWriteBuffer)[indexOfRemovedEntity * mComponentSize]);
        void* src = &((*currentWriteBuffer)[indexOfLastElement * mComponentSize]);
        memcpy(dest, src, mComponentSize);

        // Update maps
        Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
        mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        mEntityToIndexMap.erase(entity);
        mIndexToEntityMap.erase(indexOfLastElement);

        --mSize;

        // Mark the moved entity as dirty (since its data was shifted)
        mDirtyEntities.insert(entityOfLastElement);
    }

    void* GetDataForWrite(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
            "Retrieving non-existent component for write.");

        mDirtyEntities.insert(entity);

        auto currentWriteBuffer = writeBufferPtr.load();
        return &((*currentWriteBuffer)[mEntityToIndexMap[entity] * mComponentSize]);
    }

    std::vector<char>* GetWriteBuffer()
    {
        return writeBufferPtr.load();
    }

    std::vector<char>* GetReadBuffer()
    {
        return readBufferPtr.load();
    }

    void* GetData(Entity entity) override
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() &&
            "Retrieving non-existent component.");

        auto currentReadBuffer = readBufferPtr.load();
        return &((*currentReadBuffer)[mEntityToIndexMap[entity] * mComponentSize]);
    }

    void SwapData() override
    {
        if (mIsDoubleBuffered)
        {
            // Load pointers ONCE.
            // readBufferPtr should be const after init, but we'll follow your atomic load
            auto* readBuffer = readBufferPtr.load();
            auto* writeBuffer = writeBufferPtr.load();

            // Ensure buffers are the same size. This is critical.
            if (readBuffer->size() != writeBuffer->size())
            {
                readBuffer->resize(writeBuffer->size());
            }

            size_t dirtyCount = mDirtyEntities.size();

            // If most entities are dirty, just bulk copy everything
            // This copies FROM the write buffer TO the read buffer.
            if (dirtyCount > mSize / 3)
            {
                memcpy(readBuffer->data(), writeBuffer->data(), writeBuffer->size());
            }
            else if (dirtyCount > 0)
            {
                // --- Batch contiguous dirty ranges ---
                std::vector<size_t> dirtyIndices;
                dirtyIndices.reserve(dirtyCount);
                for (Entity e : mDirtyEntities)
                {
                    // Ensure entity still exists before copying
                    if (mEntityToIndexMap.count(e)) {
                        dirtyIndices.push_back(mEntityToIndexMap[e]);
                    }
                }

                if (dirtyIndices.empty()) {
                    mDirtyEntities.clear();
                    return; // No valid dirty entities
                }

                std::sort(dirtyIndices.begin(), dirtyIndices.end());

                size_t start = dirtyIndices[0];
                size_t end = start;

                for (size_t i = 1; i < dirtyIndices.size(); ++i)
                {
                    if (dirtyIndices[i] == end + 1)
                    {
                        end = dirtyIndices[i];
                    }
                    else
                    {
                        // Copy the finished contiguous block FROM Write TO Read
                        size_t count = (end - start + 1);
                        memcpy(&((*readBuffer)[start * mComponentSize]),
                            &((*writeBuffer)[start * mComponentSize]),
                            count * mComponentSize);

                        start = end = dirtyIndices[i];
                    }
                }

                // Copy the final range FROM Write TO Read
                size_t count = (end - start + 1);
                memcpy(&((*readBuffer)[start * mComponentSize]),
                    &((*writeBuffer)[start * mComponentSize]),
                    count * mComponentSize);
            }

            mDirtyEntities.clear();
        }
    }

    void MarkDirty(Entity entity)
    {
        if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
        {
            mDirtyEntities.insert(entity);
        }
	}

    void EntityDestroyed(Entity entity) override
    {
        if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
        {
            RemoveData(entity);
        }
    }

private:
    size_t mComponentSize;
    bool mIsDoubleBuffered;
    std::vector<char> mComponentData;        // Buffer A
    std::vector<char> mComponentDataSecond;  // Buffer B

    std::unordered_map<Entity, size_t> mEntityToIndexMap{};
    std::unordered_map<size_t, Entity> mIndexToEntityMap{};
    size_t mSize{};

    std::atomic<std::vector<char>*> writeBufferPtr;
    std::atomic<std::vector<char>*> readBufferPtr;

    std::unordered_set<Entity> mDirtyEntities; // Tracks modified components
};
