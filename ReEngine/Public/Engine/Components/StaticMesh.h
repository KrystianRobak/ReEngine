#pragma once

#include "ReflectionMacros.h"
#include <vector>
#include <memory>
#include <ReTypes.h>
#include <future>

class StaticMeshData;

REFCOMPONENT()
struct StaticMesh
{
	int StaticMeshId = 0;
	std::shared_ptr<StaticMeshData> StaticMeshHandler;
};