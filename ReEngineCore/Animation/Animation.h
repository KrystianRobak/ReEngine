#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Bone.h"
#include "AssimpNodeData.h"
#include "AnimHelpers.h"
#include "SkeletalMeshData.h"

static glm::mat4 aiMatrix4x4ToGlm(const aiMatrix4x4* from)
{
	glm::mat4 to;
	to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
	to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
	to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
	to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;
	return to;
}

static void extractBoneWeightForVertices(std::vector<glm::ivec4>& boneIDs_all, std::vector<glm::vec4>& weights_all, aiMesh* mesh, const aiScene* scene, SkeletalMeshData* skeletalMesh)
{
	if (!skeletalMesh) return;
	// Set the max bones to 100
	unsigned int numBones = mesh->mNumBones > 100 ? 100 : mesh->mNumBones;
	// For each bone
	for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (boneIndex >= skeletalMesh->boneInfoMap.size()) {
			skeletalMesh->boneInfoMap.push_back({ boneName, aiMatrix4x4ToGlm(&mesh->mBones[boneIndex]->mOffsetMatrix) });
			boneID = boneIndex;
			skeletalMesh->boneCount++;
		}
		else {
			for (unsigned int i = 0; i < skeletalMesh->boneInfoMap.size(); i++) {
				if (skeletalMesh->boneInfoMap[i].name == boneName) {
					boneID = i;
					break;
				}
			}
		}
		assert(boneID != -1);

		// Get all vertex weights for current bone
		aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
		unsigned int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		// For each weight at vertex x for current bone
		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			unsigned int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= boneIDs_all.size());

			// Update four most influential bones
			for (int i = 0; i < 4; ++i)
			{
				if (boneIDs_all[vertexId][i] < 0)
				{
					weights_all[vertexId][i] = weight;
					boneIDs_all[vertexId][i] = boneID;
					break;
				}
			}
		}
	}
}


class Animation
{
public:

	Animation(const std::string& animationPath, SkeletalMeshData* model)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		assert(scene && scene->mRootNode);
		if (scene->mNumAnimations == 0)
			return;
		aiAnimation* animation = scene->mAnimations[0];
		duration = (float)animation->mDuration;
		tps = (float)animation->mTicksPerSecond;
		generateBoneTree(&rootNode, scene->mRootNode);
		// Reset all root transformations
		rootNode.transformation = glm::mat4(1.0f);
	}

	Animation() {

		rootNode.transformation = glm::mat4(1.0f);
	};

	Animation(const std::string& animationPath, aiScene* scene)
	{
		generateBoneTree(&rootNode, scene->mRootNode);
		// Reset all root transformations
		rootNode.transformation = glm::mat4(1.0f);
	}

	Bone* findBone(const std::string& name) {
		if (!indexMapBuilt_) BuildBoneIndexMap();

		auto it = boneNameToIndex_.find(name);
		if (it != boneNameToIndex_.end()) {
			return &bones[it->second];
		}
		return nullptr;
	}

	void LoadIntermediateBones(SkeletalMeshData* skeletalMesh)
	{
		if (!skeletalMesh) return;

		// 1. Get a reference to the Mesh's master bone list
		auto& meshBoneInfo = skeletalMesh->boneInfoMap;

		// 2. Iterate over all bones driven by this animation
		for (auto& animBone : bones)
		{
			std::string boneName = animBone.getBoneName();
			int boneId = -1;

			// A. Try to find this bone in the existing Mesh list
			for (unsigned int i = 0; i < meshBoneInfo.size(); i++) {
				if (meshBoneInfo[i].name == boneName) {
					boneId = i;
					break;
				}
			}

			// B. If the Mesh doesn't know this bone (it's an intermediate/structural node),
			//    we must add it to the Mesh now so the hierarchy remains unbroken.
			if (boneId == -1) {
				BoneProps newProp;
				newProp.name = boneName;

				// Intermediate bones usually don't affect the skin (no weights),
				// so we set their offset matrix to Identity.
				newProp.offset = glm::mat4(1.0f);

				meshBoneInfo.push_back(newProp);

				// The new ID is the index we just added
				boneId = (int)meshBoneInfo.size() - 1;

				// Update the mesh's bone count if you use it for loop limits
				skeletalMesh->boneCount++;
			}

			// C. CRITICAL: Tell the Animation Bone what its real ID is
			animBone.SetID(boneId);
		}

		// 3. [FIX] Copy the updated list back to the Animation's local storage
		// This ensures getBoneProps() returns the correct full list, including
		// the new intermediate bones we just added.
		this->boneProps = meshBoneInfo;
	}

	void SetDuration(float duration_) { duration = duration_; }
	void SetTicksPerSecond(float tps_) { tps = tps_; }
	void SetRootNode(const AssimpNodeData& node_) { rootNode = node_; }

	void AddBone(const Bone& bone) {
		bones.push_back(bone);
	}

	inline float getTicksPerSecond() { return tps; }

	inline float getDuration() { return duration; }

	inline const AssimpNodeData* getRootNode() { return &rootNode; }

	inline const std::vector<BoneProps>& getBoneProps()
	{
		return boneProps;
	}


private:
	float duration = 0.0f;
	float tps = 0.0f;
	std::vector<Bone> bones;
	AssimpNodeData rootNode;
	std::vector<BoneProps> boneProps;
	std::unordered_map<std::string, int> boneNameToIndex_;
	bool indexMapBuilt_ = false;
	


	void BuildBoneIndexMap() {
		if (indexMapBuilt_) return;

		for (size_t i = 0; i < bones.size(); ++i) {
			boneNameToIndex_[bones[i].getBoneName()] = i;
		}
		indexMapBuilt_ = true;
	}

	void generateBoneTree(AssimpNodeData* parent, const aiNode* src)
	{
		assert(src);

		parent->name = src->mName.data;
		parent->transformation = aiMatrix4x4ToGlm(&src->mTransformation);
		parent->childrenCount = src->mNumChildren;

		for (unsigned int i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			generateBoneTree(&newData, src->mChildren[i]);
			parent->children.push_back(newData);
		}
	}
};