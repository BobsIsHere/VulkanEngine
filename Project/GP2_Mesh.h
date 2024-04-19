#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Vertex.h"
#include "GP2_Shader.h"
#include "GP2_Buffer.h"
#include "vulkanbase/VulkanUtil.h"

class GP2_Mesh final
{
public:
	GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice);
	~GP2_Mesh() = default;

	void Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void DestroyMesh();
	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);

	void AddVertex(const glm::vec2 pos, const glm::vec3 color);
	void AddIndices(const std::vector<uint16_t> indices); 

	void MakeTriangle();

private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDevice m_Device; 
	VkPhysicalDevice m_PhysicalDevice; 

	GP2_Buffer* m_pVertexBuffer;
	GP2_Buffer* m_pIndexBuffer;
	
	std::vector<Vertex> m_MeshVertices; 
	std::vector<uint16_t> m_MeshIndices;

	MeshData m_VertexConstant; 
};
