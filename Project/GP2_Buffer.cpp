#include "GP2_Buffer.h"
#include "GP2_CommandBuffer.h"
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanBase.h"

GP2_Buffer::GP2_Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size) :
    m_Device{ device },
    m_Size{ size },
    m_VkBuffer{},
    m_VkBufferMemory{} 
{
    // Create the buffer
    VkBufferCreateInfo bufferInfo{}; 
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; 
    bufferInfo.size = size; 
    bufferInfo.usage = usage; 
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &m_VkBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create buffer!"); 
    }

    // Allocate memory for the buffer
    VkMemoryRequirements memRequirements; 
    vkGetBufferMemoryRequirements(device, m_VkBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{}; 
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
    allocInfo.allocationSize = memRequirements.size; 
    allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties); 

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_VkBufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to allocate buffer memory!"); 
    }

    // Bind buffer memory
    vkBindBufferMemory(device, m_VkBuffer, m_VkBufferMemory, 0);
}

GP2_Buffer::~GP2_Buffer()
{
    Destroy();
}

//transfer to device local as name?
void GP2_Buffer::Upload(VkDeviceSize size, void* data)
{
    void* mappedData{};
    vkMapMemory(m_Device, m_VkBufferMemory, 0, size, 0, &mappedData);

    // Copy data to mapped memory
    memcpy(mappedData, data, static_cast<size_t>(size));

    // Unmap buffer memory
    vkUnmapMemory(m_Device, m_VkBufferMemory);
}

//make visible or make host visible as name?
void GP2_Buffer::Map(void** data)
{
    vkMapMemory(m_Device, m_VkBufferMemory, 0, m_Size, 0, data);
}

void GP2_Buffer::Destroy()
{
	vkDestroyBuffer(m_Device, m_VkBuffer, nullptr);
	vkFreeMemory(m_Device, m_VkBufferMemory, nullptr);
}

void GP2_Buffer::BindAsVertexBuffer(VkCommandBuffer commandBuffer)
{
    VkBuffer vertexBuffers[] = { m_VkBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
}

void GP2_Buffer::BindAsIndexBuffer(VkCommandBuffer commandBuffer)
{
    vkCmdBindIndexBuffer(commandBuffer, m_VkBuffer, 0, VK_INDEX_TYPE_UINT16);
}

uint32_t GP2_Buffer::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
