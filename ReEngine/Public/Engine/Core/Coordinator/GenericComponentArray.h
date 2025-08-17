// In a new file or alongside ComponentArray.h

#include "ReTypes.h"
#include <vector>
#include <unordered_map>
#include <cassert>
#include <cstring> // For memcpy

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
    virtual void EntityDestroyed(Entity entity) = 0;
    virtual void* GetData(Entity entity) = 0;
};

class GenericComponentArray : public IComponentArray
{
public:
    // Constructor takes the size of the component type
    GenericComponentArray(size_t componentSize) : mComponentSize(componentSize) {}

    void InsertData(Entity entity, void* componentData)
    {
        assert(mEntityToIndexMap.find(entity) == mEntityToIndexMap.end() && "Component added to same entity more than once.");

        size_t newIndex = mSize;
        mEntityToIndexMap[entity] = newIndex;
        mIndexToEntityMap[newIndex] = entity;

        // Resize the buffer if needed
        if ((newIndex + 1) * mComponentSize > mComponentData.size()) {
            mComponentData.resize((newIndex + 1) * mComponentSize);
        }

        // Copy the component data into our byte array
        memcpy(&mComponentData[newIndex * mComponentSize], componentData, mComponentSize);

        ++mSize;
    }

    void RemoveData(Entity entity)
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Removing non-existent component.");

        size_t indexOfRemovedEntity = mEntityToIndexMap[entity];
        size_t indexOfLastElement = mSize - 1;

        // Move the last element's data into the removed element's slot
        void* dest = &mComponentData[indexOfRemovedEntity * mComponentSize];
        void* src = &mComponentData[indexOfLastElement * mComponentSize];
        memcpy(dest, src, mComponentSize);

        // Update the maps to point to the new location
        Entity entityOfLastElement = mIndexToEntityMap[indexOfLastElement];
        mEntityToIndexMap[entityOfLastElement] = indexOfRemovedEntity;
        mIndexToEntityMap[indexOfRemovedEntity] = entityOfLastElement;

        mEntityToIndexMap.erase(entity);
        mIndexToEntityMap.erase(indexOfLastElement);

        --mSize;
    }

    // Returns a raw pointer to the component data
    void* GetData(Entity entity) override
    {
        assert(mEntityToIndexMap.find(entity) != mEntityToIndexMap.end() && "Retrieving non-existent component.");
        return &mComponentData[mEntityToIndexMap[entity] * mComponentSize];
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
    std::vector<char> mComponentData; // Raw byte buffer for all components
    std::unordered_map<Entity, size_t> mEntityToIndexMap{};
    std::unordered_map<size_t, Entity> mIndexToEntityMap{};
    size_t mSize{};
};