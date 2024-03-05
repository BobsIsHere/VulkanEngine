#pragma once
#include "GP2_CommandBuffer.h"

struct QueueFamilyIndices;

class GP2_CommandPool
{
public:
	GP2_CommandPool();
	~GP2_CommandPool() = default;

	void Initialize(const VkDevice& device, const QueueFamilyIndices& queue);
	void Destroy();

	GP2_CommandBuffer CreateCommandBuffer() const;

private:
	VkCommandPool m_CommandPool;
	VkDevice m_VkDevice;
};
