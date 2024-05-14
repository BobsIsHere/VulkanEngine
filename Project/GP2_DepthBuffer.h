#pragma once
#include "vulkanbase/VulkanUtil.h"
#include "GP2_CommandPool.h" 

static VkFormat FindSupportedFormat(const VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props{};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

class GP2_DepthBuffer final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_DepthBuffer();
	~GP2_DepthBuffer();

	//-----------
	// Functions
	//-----------
	void Initialize(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool); 
	void CreateDepthResources();
	VkImageView GetDepthImageView() const { return m_DepthImageView; }
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

