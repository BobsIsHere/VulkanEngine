#pragma once
#include <vector>

#include "Vertex.h"
#include "GP2_CommandPool.h"
#include "vulkan/vulkan_core.h"

class GP2_Buffer final
{
public:
	GP2_Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size);
	~GP2_Buffer();

	void TransferDeviceLocal(void* data);
	void Map(void** data);
	void CopyBuffer(GP2_Buffer srcBuffer, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);

	void Destroy();
	
	void BindAsVertexBuffer(VkCommandBuffer commandBuffer);
	void BindAsIndexBuffer(VkCommandBuffer commandBuffer);

	VkBuffer GetVkBuffer() const { return m_VkBuffer; }
	VkDeviceSize GetSizeInBytes() const { return m_Size; }
	
private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDevice m_Device;
	VkDeviceSize m_Size; 
	VkBuffer m_VkBuffer;
	VkDeviceMemory m_VkBufferMemory;
};
