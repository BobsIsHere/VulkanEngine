#pragma once
#include <vector>

#include "Vertex.h"
#include "GP2_Shader.h"
#include "GP2_CommandPool.h"
#include "vulkan/vulkan_core.h"

class GP2_Buffer 
{
public:
	GP2_Buffer(VkDevice device, VkPhysicalDevice physicalDevice); 
	~GP2_Buffer();

	void CreateVertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void CreateIndexBuffer(const std::vector<uint16_t> indices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void BindBuffers(VkCommandBuffer commandBuffer, VkBuffer vertexBuffers[], VkDeviceSize offsets[]);

	void DestroyVertexBuffer();
	void DestroyIndexBuffer();

	VkBuffer GetVertexBuffer() const { return m_VertexBuffer; }
	VkDeviceMemory GetVertexBufferMemory() const { return m_VertexBufferMemory; }
	
private:
	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;

	VkBuffer m_VertexBuffer;
	VkDeviceMemory m_VertexBufferMemory;

	VkBuffer m_IndexBuffer;
	VkDeviceMemory m_IndexBufferMemory;
};
