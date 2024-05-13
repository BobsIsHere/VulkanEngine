#pragma once
#include "vulkanbase/VulkanUtil.h"
#include "GP2_CommandPool.h" 

class GP2_DepthBuffer final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_DepthBuffer(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool); 
	~GP2_DepthBuffer();

	//-----------
	// Functions
	//-----------
	void CreateDepthResources();
	VkImageView GetDepthImageView() const { return m_DepthImageView; }
	VkFormat FindDepthFormat(); 
	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

private:
	//-----------
	// Functions
	//-----------
	VkCommandBuffer BeginSingleTimeCommands(); 
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer); 

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	bool HasStencilComponent(VkFormat format); 
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//-----------
	// Variables
	//-----------
	VulkanContext m_VulkanContext;
	GP2_CommandPool m_CommandPool;

	VkQueue m_GraphicsQueue;
	VkImage m_DepthImage;
	VkDeviceMemory m_DepthImageMemory;
	VkImageView m_DepthImageView;
};
