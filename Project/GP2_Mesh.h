#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "GP2_Shader.h"
#include "GP2_Buffer.h"
#include "vulkanbase/VulkanUtil.h"

class GP2_Mesh
{
public:
	GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice);
	~GP2_Mesh() = default;

	void Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void DestroyMesh();
	void Draw(VkCommandBuffer buffer);

	void AddVertex(const glm::vec2 pos, const glm::vec3 color);
	void AddIndices(const std::vector<uint16_t> indices); 

	bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, bool flipAxisAndWinding = true);

private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::unique_ptr<GP2_Buffer> m_pBuffer;

	VkDevice m_Device; 
	VkPhysicalDevice m_PhysicalDevice; 
	
	std::vector<Vertex> m_MeshVertices; 
	std::vector<uint16_t> m_MeshIndices;
};
