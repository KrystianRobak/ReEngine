#pragma once


#include "GL/glew.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <MeshData.h>


struct GPUBuffers {
	unsigned int VAO;
	std::vector<unsigned int> VBOs; 
	unsigned int EBO;
};

inline void computeTangentBasis(
	std::vector<glm::vec3>& vertices,
	std::vector<glm::vec2>& uvs,
	std::vector<glm::vec3>& normals,
	std::vector<glm::vec3>& tangents,
	std::vector<glm::vec3>& bitangents)
{
	for (size_t i = 0; i < vertices.size() - 2; i += 3)
	{

		
		glm::vec3& v0 = vertices[i + 0];
		glm::vec3& v1 = vertices[i + 1];
		glm::vec3& v2 = vertices[i + 2];

		
		glm::vec2& uv0 = uvs[i + 0];
		glm::vec2& uv1 = uvs[i + 1];
		glm::vec2& uv2 = uvs[i + 2];

		
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

		
		
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
	}
}

template <class T>
unsigned int generateAttribute(int id, int elementsPerEntry, std::vector<T> data, bool normalize, bool integer = false)
{
	unsigned int bufferID;
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_ARRAY_BUFFER, bufferID);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), GL_STATIC_DRAW);
	if (integer)
		glVertexAttribIPointer(id, elementsPerEntry, GL_INT, sizeof(T), 0);
	else
		glVertexAttribPointer(id, elementsPerEntry, GL_FLOAT, normalize ? GL_TRUE : GL_FALSE, sizeof(T), 0);
	glEnableVertexAttribArray(id);
	return bufferID;
}

inline GPUBuffers generateBuffer(MeshData& mesh)
{
	GPUBuffers buffers;

	
	glGenVertexArrays(1, &buffers.VAO);
	glBindVertexArray(buffers.VAO);

	
	
	buffers.VBOs.push_back(generateAttribute(0, 3, mesh.vertices, false));

	if (mesh.Normals.size() > 0)
	{
		buffers.VBOs.push_back(generateAttribute(1, 3, mesh.Normals, true));
	}

	if (mesh.TexCoords.size() > 0)
	{
		buffers.VBOs.push_back(generateAttribute(2, 2, mesh.TexCoords, false));

		
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;

		if (mesh.Tangents.size() == 0)
		{
			computeTangentBasis(mesh.vertices, mesh.TexCoords, mesh.Normals, tangents, bitangents);
		}
		else
		{
			tangents = mesh.Tangents;
			bitangents = mesh.Bitangents;
		}

		buffers.VBOs.push_back(generateAttribute(3, 3, tangents, false));
		buffers.VBOs.push_back(generateAttribute(4, 3, bitangents, false));
	}

	if (mesh.boneIDs.size() > 0)
	{
		buffers.VBOs.push_back(generateAttribute(5, 4, mesh.boneIDs, false, true));
		buffers.VBOs.push_back(generateAttribute(6, 4, mesh.weights, false));
	}

	
	glGenBuffers(1, &buffers.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(), GL_STATIC_DRAW);

	
	glBindVertexArray(0);

	return buffers;
}

inline void generateDepthMap(unsigned int& depthMap, unsigned int& FBO, unsigned int width, unsigned int height) {
	glGenFramebuffers(1, &FBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


