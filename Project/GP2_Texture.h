#pragma once
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanBase.h"

#include "GP2_CommandPool.h"

class GP2_Texture final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_Texture(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool);
	~GP2_Texture();

	//-----------
	// Functions
	//-----------
	void Initialize(const char* filePath);
	void CleanUp();

	void CreateTextureImage(const char* filePath);
	void CreateTextureImageView();
	void CreateTextureSampler();

	static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkImageView GetTextureImageView() const { return m_TextureImageView; }
	VkSampler GetTextureSampler() const { return m_TextureSampler; }

private:
	//-----------
	// Functions
	//-----------
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	bool HasStencilComponent(VkFormat format);

	//-----------
	// Variables
	//-----------
	VulkanContext m_VulkanContext;
	GP2_CommandPool m_CommandPool;
	VkQueue m_GraphicsQueue;

	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkSampler m_TextureSampler;
};
