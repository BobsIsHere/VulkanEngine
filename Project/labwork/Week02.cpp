#include <stb_image.h>
#include "vulkanbase/VulkanBase.h"
#include "GP2_Shader.h"

QueueFamilyIndices VulkanBase::FindQueueFamilies(VkPhysicalDevice device) 
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

		if (presentSupport) 
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete()) 
		{
			break;
		}

		i++;
	}

	return indices;
}

VkCommandBuffer VulkanBase::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{}; 
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	allocInfo.commandPool = m_CommandPool.GetVkCommandPool();  
	allocInfo.commandBufferCount = 1; 

	VkCommandBuffer commandBuffer; 
	vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{}; 
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; 
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; 

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer; 
}

void VulkanBase::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer); 

	VkSubmitInfo submitInfo{}; 
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; 
	submitInfo.commandBufferCount = 1; 
	submitInfo.pCommandBuffers = &commandBuffer; 

	vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_GraphicsQueue);

	vkFreeCommandBuffers(m_Device, m_CommandPool.GetVkCommandPool(), 1, &commandBuffer);
}

void VulkanBase::CreateTextureImage()
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("resources/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) 
	{
		throw std::runtime_error("failed to load texture image!");
	}

	GP2_Buffer stagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, imageSize };

	stagingBuffer.TransferDeviceLocal(pixels);

	stbi_image_free(pixels);

	CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, 
			    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			 m_TextureImage, m_TextureImageMemory);
}

void VulkanBase::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1; 
	imageInfo.mipLevels = 1; 
	imageInfo.arrayLayers = 1; 
	imageInfo.format = format; 
	imageInfo.tiling = tiling; 
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 
	imageInfo.usage = usage; 
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; 
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 

	if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}
}
