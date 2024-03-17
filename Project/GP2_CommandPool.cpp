#include "GP2_CommandPool.h"
#include "vulkanbase/VulkanBase.h"

GP2_CommandPool::GP2_CommandPool() :
	m_CommandPool{ VK_NULL_HANDLE },
	m_VkDevice{ VK_NULL_HANDLE }
{

}

void GP2_CommandPool::Initialize(const VkDevice& device, const QueueFamilyIndices& queue)
{
	m_VkDevice = device;

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queue.graphicsFamily.value();

	if (vkCreateCommandPool(m_VkDevice, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void GP2_CommandPool::Destroy()
{
	vkDestroyCommandPool(m_VkDevice, m_CommandPool, nullptr);
}

GP2_CommandBuffer GP2_CommandPool::CreateCommandBuffer() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer{};

	if (vkAllocateCommandBuffers(m_VkDevice, &allocInfo, &commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	GP2_CommandBuffer cmdBuffer{};
	cmdBuffer.SetVkCommandBuffer(commandBuffer);

	return cmdBuffer; 
}

VkCommandPool GP2_CommandPool::GetVkCommandPool() const
{
	return m_CommandPool;
}
