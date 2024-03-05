#pragma once
#include "vulkan/vulkan_core.h"

class GP2_CommandBuffer
{
public:
	GP2_CommandBuffer();
	~GP2_CommandBuffer() = default;

	void SetVkCommandBuffer(VkCommandBuffer buffer);
	VkCommandBuffer GetVkCommandBuffer() const;

	void Reset() const;
	void BeginRecording() const;
	void EndRecording() const;

	void Sumbit(VkSubmitInfo& info) const;

private:
	VkCommandBuffer m_CommandBuffer;
};
