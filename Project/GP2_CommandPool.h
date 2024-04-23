#pragma once
#include "GP2_CommandBuffer.h"

struct QueueFamilyIndices;

class GP2_CommandPool final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_CommandPool();
	~GP2_CommandPool() = default;

	//-----------
	// Functions
	//-----------
	void Initialize(const VkDevice& device, const QueueFamilyIndices& queue);
	void Destroy();

	GP2_CommandBuffer CreateCommandBuffer() const;
	VkCommandPool GetVkCommandPool() const;

private:
	//-----------
	// Variables
	//-----------
	VkCommandPool m_CommandPool;
	VkDevice m_VkDevice;
};
