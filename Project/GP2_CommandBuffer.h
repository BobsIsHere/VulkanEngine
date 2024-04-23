#pragma once
#include "vulkan/vulkan_core.h"

class GP2_CommandBuffer final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_CommandBuffer();
	~GP2_CommandBuffer() = default;

	//-----------
	// Functions
	//-----------
	void SetVkCommandBuffer(VkCommandBuffer buffer);
	VkCommandBuffer GetVkCommandBuffer() const;

	void Reset() const;
	void BeginRecording(VkCommandBufferUsageFlags flags) const;
	void EndRecording() const;

	void Sumbit(VkSubmitInfo& info) const;

private:
	//-----------
	// Variables
	//-----------
	VkCommandBuffer m_CommandBuffer;
};
