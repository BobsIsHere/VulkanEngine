#include "GP2_Buffer.h"
#include "GP2_CommandBuffer.h"
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanBase.h"

GP2_Buffer::GP2_Buffer(VkDevice device, VkPhysicalDevice physicalDevice) :
    m_Device{ device },
    m_PhysicalDevice{ physicalDevice },
    m_IndexBuffer{},
    m_IndexBufferMemory{},
    m_VertexBuffer{},
    m_VertexBufferMemory{}
{

}

GP2_Buffer::~GP2_Buffer()
{
}

void GP2_Buffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements{};
    vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

void GP2_Buffer::CreateVertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size(); 

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              stagingBuffer, stagingBufferMemory);

    void* data{};
    vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0 ,&data);
    memcpy(data, vertices.data(), size_t(bufferSize));
    vkUnmapMemory(m_Device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              m_VertexBuffer, m_VertexBufferMemory);

    CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize, graphicsQueue, queueFamilyIndices);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void GP2_Buffer::CreateIndexBuffer(const std::vector<uint32_t> indices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory);

    void* data{};
    vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_Device, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

    CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize, graphicsQueue, queueFamilyIndices);

    vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
    vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void GP2_Buffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
    GP2_CommandPool commandPool{};
    commandPool.Initialize(m_Device, queueFamilyIndices); 

    GP2_CommandBuffer commandBuffer{ commandPool.CreateCommandBuffer() }; 

    commandBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

   VkBufferCopy copyRegion{};
   copyRegion.srcOffset = 0;
   copyRegion.dstOffset = 0;
   copyRegion.size = size; 
   vkCmdCopyBuffer(commandBuffer.GetVkCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

   commandBuffer.EndRecording();

    VkSubmitInfo submitInfo{}; 
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; 
    commandBuffer.Sumbit(submitInfo);
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); 
    vkQueueWaitIdle(graphicsQueue); 

    const VkCommandBuffer commandBufferVk = commandBuffer.GetVkCommandBuffer();
    vkFreeCommandBuffers(m_Device, commandPool.GetVkCommandPool(), 1, &commandBufferVk);

    commandPool.Destroy();
}

void GP2_Buffer::BindBuffers(VkCommandBuffer commandBuffer, VkBuffer vertexBuffers[], VkDeviceSize offsets[])
{
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
}

void GP2_Buffer::DestroyVertexBuffer()
{
    vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
}

void GP2_Buffer::DestroyIndexBuffer()
{
    vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
    vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
}

uint32_t GP2_Buffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
    {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}
