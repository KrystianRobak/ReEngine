#pragma once
#include <future>
#include <memory>
#include <string>
#include <ReTypes.h>

class StaticMeshData;
class PendingStaticMesh;
struct TextureData;

class AssetManagerApi
{
public:
	virtual std::future<std::shared_ptr<StaticMeshData>> loadFBX(const std::string& path) = 0;

	virtual void AddPendingMesh(Entity entity, std::future<std::shared_ptr<StaticMeshData>> future) = 0;

	virtual std::future<std::shared_ptr<TextureData>> loadTexture(const std::string& path) = 0;

	virtual std::vector<std::string> GetCachedPaths() = 0;

	virtual std::vector<PendingStaticMesh>& GetPendingMeshes() = 0;
};