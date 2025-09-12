// GenericComponentArray.h (Corrected)

#include "ReTypes.h"
#include <vector>
#include <atomic>
#include <unordered_map>
#include <cassert>
#include <cstring> // For memcpy

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
    GenericComponentArray(size_t componentSize, bool IsDoubleBuffered) : mComponentSize(componentSize),
        mIsDoubleBuffered(IsDoubleBuffered)
    {
        if(mIsDoubleBuffered)
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
        assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() && "Component added to same entity more than once.");

        size_t newIndex = mSize;
        mEntityToIndexMap[entity] = newIndex;
        mIndexToEntityMap[newIndex] = entity;

        // Get the current write buffer
        auto currentWriteBuffer = writeBufferPtr.load();

        // Resize both buffers to keep their sizes in sync, but only write to one.
        if ((newIndex + 1) * mComponentSize > currentWriteBuffer->size()) {
            mComponentData.resize((newIndex + 1) * mComponentSize);
            if (mIsDoubleBuffered) {
                mComponentDataSecond.resize((newIndex + 1) * mComponentSize);
            }
        }

        // **FIX:** Only copy data into the current write buffer.
        // The read buffer is left untouched to allow safe reading from another thread.
        memcpy(&((*currentWriteBuffer)[newIndex * mComponentSize]), componentData, mComponentSize);

        ++mSize;
    }

    void RemoveData(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Removing non-existent component.");

        size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
        size_t indexOfLastElement = mSize - 1;

        // Get the current write buffer
        auto currentWriteBuffer = writeBufferPtr.load();

        // **FIX:** Only move data within the write buffer.
        // The data in the read buffer remains valid for the current frame.
        void* dest = &((*currentWriteBuffer)[indexOfRemovedEntity * mComponentSize]);
        void* src = &((*currentWriteBuffer)[indexOfLastElement * mComponentSize]);
        memcpy(dest, src, mComponentSize);

        // Update the maps to point to the new location. This is shared state.
        Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
        mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        mEntityToIndexMap.erase(entity);
        mIndexToEntityMap.erase(indexOfLastElement);

        --mSize;
    }

    std::vector<char>* GetWriteBuffer()
    {
        return writeBufferPtr.load();
    }

    std::vector<char>* GetReadBuffer()
    {
        return readBufferPtr.load();
    }

    // Returns a raw pointer to the component data from the READ buffer
    void* GetData(Entity entity) override
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Retrieving non-existent component.");

        // **FIX:** This must read from the buffer pointed to by readBufferPtr.
        auto currentReadBuffer = readBufferPtr.load();
        return &((*currentReadBuffer)[mEntityToIndexMap[entity] * mComponentSize]);
    }

    void SwapData()
    {
        // This atomic swap is correct. No changes needed here.
        if (mIsDoubleBuffered)
        {
            auto oldWritePtr = writeBufferPtr.exchange(readBufferPtr.load());
            readBufferPtr.store(oldWritePtr);
        }
    }

    void EntityDestroyed(Entity entity) override
    {
        if (mEntityToIndexMap.find(entity) != mEntityToIndexMap.end())
        {
            // Note: This stages the removal for the next frame's write buffer.
            RemoveData(entity);
        }
    }

private:
    size_t mComponentSize;
    bool mIsDoubleBuffered;
    std::vector<char> mComponentData; // Acts as Buffer A
    std::vector<char> mComponentDataSecond; // Acts as Buffer B

    // Mappings are shared and represent the state of the *next* frame.
    std::unordered_map<Entity, size_t> mEntityToIndexMap{};
    std::unordered_map<size_t, Entity> mIndexToEntityMap{};
    size_t mSize{};

    std::atomic<std::vector<char>*> writeBufferPtr;
    std::atomic<std::vector<char>*> readBufferPtr;
};