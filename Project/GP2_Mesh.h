#pragma once
#include <glm/glm.hpp>
#include <GP2_Shader.h>
#include <vector>
#include "vulkanbase/VulkanUtil.h"

class GP2_Mesh
{
public:
	GP2_Mesh();
	~GP2_Mesh() = default;

	void Initialize(VkPhysicalDevice physicalDevice, VkDevice device);
	void DestroyMesh(VkDevice device);
	void Draw(VkCommandBuffer buffer);
	void AddVertex(glm::vec2 pos, glm::vec3 color);

private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;
	std::vector<Vertex> m_MeshVertices; 
};
