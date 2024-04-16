#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>

#include "Vertex.h"
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
	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);

	void AddVertex(const glm::vec2 pos, const glm::vec3 color);
	void AddIndices(const std::vector<uint16_t> indices); 

	//bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, bool flipAxisAndWinding = true);

	//BUFFER FUNCTIONS
	void CreateVertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void CreateIndexBuffer(const std::vector<uint16_t> indices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);

private:
	//BUFFER FUNCTIONS
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory); 
	void DestroyVertexBuffer();
	void DestroyIndexBuffer();

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkBuffer m_VkVertexBuffer;
	VkDeviceMemory m_VkVertexBufferMemory;

	VkBuffer m_VkIndexBuffer;
	VkDeviceMemory m_VkIndexBufferMemory;

	VkDevice m_Device; 
	VkPhysicalDevice m_PhysicalDevice; 

	std::unique_ptr<GP2_Buffer> m_pVertexBuffer;
	std::unique_ptr<GP2_Buffer> m_pIndexBuffer;
	
	std::vector<Vertex> m_MeshVertices; 
	std::vector<uint16_t> m_MeshIndices;

	const void* m_VertexConstant; 
};
