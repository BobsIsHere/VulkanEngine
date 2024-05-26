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
	void Initialize(const char* filePath, QueueFamilyIndices queueFamInd);
	void CleanUp();

	void CreateTextureImageView();
	void CreateTextureSampler();

	void LoadImageData(const std::string& filePath);

	static VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	void CreateImage(VkFormat format);
	void TransitionImageLayout(QueueFamilyIndices queueFamInd, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkImageView GetTextureImageView() const { return m_TextureImageView; }
	VkSampler GetTextureSampler() const { return m_TextureSampler; }

private:
	//-----------
	// Functions
	//-----------
	void CopyBufferToImage(QueueFamilyIndices queueFamInd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	bool HasStencilComponent(VkFormat format);

	//-----------
	// Variables
	//-----------
	VulkanContext m_VulkanContext;
	GP2_CommandPool m_CommandPool;
	VkQueue m_GraphicsQueue;

	int m_TextureWidth; 
	int m_TextureHeight;
	int m_TextureChannels;

	GP2_Buffer* m_StagingBuffer;

	VkImage m_TextureImage;
	VkDeviceMemory m_TextureImageMemory;
	VkImageView m_TextureImageView;
	VkSampler m_TextureSampler;
};
