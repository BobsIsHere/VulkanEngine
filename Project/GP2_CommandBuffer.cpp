#include "GP2_CommandBuffer.h"
#include <stdexcept>

GP2_CommandBuffer::GP2_CommandBuffer() :
	m_CommandBuffer{}
{
}

void GP2_CommandBuffer::SetVkCommandBuffer(VkCommandBuffer buffer)
{
	m_CommandBuffer = buffer;
}

VkCommandBuffer GP2_CommandBuffer::GetVkCommandBuffer() const
{
	return m_CommandBuffer;
}

void GP2_CommandBuffer::Reset() const
{
	vkResetCommandBuffer(m_CommandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
}

void GP2_CommandBuffer::BeginRecording(VkCommandBufferUsageFlags flags) const
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = flags; //optional
	beginInfo.pInheritanceInfo = nullptr; //optional

	if (vkBeginCommandBuffer(m_CommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}
}

void GP2_CommandBuffer::EndRecording() const
{
	if (vkEndCommandBuffer(m_CommandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}
}

void GP2_CommandBuffer::Sumbit(VkSubmitInfo& info) const
{
	info.commandBufferCount = 1;
	info.pCommandBuffers = &m_CommandBuffer;
}
