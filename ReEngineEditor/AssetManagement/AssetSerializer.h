#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include "ReEngineExport.h"
#include "StaticMeshData.h"
#include "SkeletalMeshData.h"
#include "AssetFileFormat.h"



template<typename T>
void WriteVector(std::ofstream& out, const std::vector<T>& vec) {
    uint32_t size = static_cast<uint32_t>(vec.size());
    out.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
    if (size > 0) {
        out.write(reinterpret_cast<const char*>(vec.data()), size * sizeof(T));
    }
}

// Helper to read vector data directly
template<typename T>
void ReadVector(std::ifstream& in, std::vector<T>& vec) {
    uint32_t size = 0;
    in.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
    vec.resize(size);
    if (size > 0) {
        in.read(reinterpret_cast<char*>(vec.data()), size * sizeof(T));
    }
}


class ENGINE_API AssetSerializer {
public:
    // --- COOKING (Save to disk) ---

    static bool SaveStaticMesh(const std::string& path, const StaticMeshData& data);

    static std::shared_ptr<StaticMeshData> LoadStaticMesh(const std::string& path);

    static bool SaveSkeletalMesh(const std::string& path, const SkeletalMeshData& data);

    static std::shared_ptr<SkeletalMeshData> LoadSkeletalMesh(const std::string& path);

	static bool ImportFile(const std::string& path, const std::string& destination);

};


