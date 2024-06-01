#pragma once
#include <stdexcept>
#include <vector>
#include "Vertex.h"
#include "GP2_Buffer.h"
#include "vulkan/vulkan_core.h"
#include "vulkanbase/VulkanUtil.h"
#include "vulkanbase/VulkanBase.h"

template <class UBO>
class GP2_DescriptorPool final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_DescriptorPool(VkDevice device, size_t count, size_t imageCount = 0);
	~GP2_DescriptorPool();

	//-----------
	// Functions
	//-----------
	void Initialize(const VulkanContext& context, const std::vector<std::pair<VkImageView, VkSampler>>& textureImageViewsSamplers = {});

	void SetUBO(UBO data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout()
	{
		return m_DescriptorSetLayout;
	}

	void CreateDescriptorSets(const std::vector<std::pair<VkImageView, VkSampler>>& textureImageViewsSamplers = {});

	void BindDescriptorSet(VkCommandBuffer buffer, VkPipelineLayout layout, size_t index);

private:
	//-----------
	// Functions
	//-----------
	void CreateDescriptorSetLayout(const VulkanContext& context, size_t imageCount = 0); 
	void CreateUBOs(const VulkanContext& context);

	//-----------
	// Variables
	//-----------
	VkDevice m_Device;
	VkDeviceSize m_Size;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;

	std::vector<GP2_Buffer*> m_UBOs;
	std::vector<void*> m_UBOsMapped;

	size_t m_Count;
};

template<class UBO>
GP2_DescriptorPool<UBO>::GP2_DescriptorPool(VkDevice device, size_t count, size_t imageCount) :
	m_Device{ device },
	m_Size{ sizeof(VertexUBO) },
	m_Count(count),
	m_DescriptorPool{ nullptr },
	m_DescriptorSetLayout{ nullptr }
{
	std::vector<VkDescriptorPoolSize> poolSizes(imageCount + 1);  
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_Count);
	
	for (size_t idx = 1; idx < imageCount + 1; ++idx)
	{
		poolSizes[idx].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
		poolSizes[idx].descriptorCount = static_cast<uint32_t>(m_Count);
	}

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(m_Count);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

template<class UBO>
GP2_DescriptorPool<UBO>::~GP2_DescriptorPool()
{
	for (size_t i = 0; i < m_Count; ++i)
	{
		m_UBOs[i]->Destroy();
	}

	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::Initialize(const VulkanContext& context, const std::vector<std::pair<VkImageView, VkSampler>>& textureImageViewsSamplers)
{
	CreateDescriptorSetLayout(context, textureImageViewsSamplers.size());
	CreateUBOs(context);
	CreateDescriptorSets(textureImageViewsSamplers);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSets(const std::vector<std::pair<VkImageView, VkSampler>>& textureImageViewsSamplers)
{
	std::vector<VkDescriptorSetLayout> layouts(m_Count, m_DescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_DescriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_Count);
	allocInfo.pSetLayouts = layouts.data();

	m_DescriptorSets.resize(m_Count);

	if (vkAllocateDescriptorSets(m_Device, &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	std::vector<VkDescriptorImageInfo> imageInfos(textureImageViewsSamplers.size());
	for (size_t texIdx = 0; texIdx < textureImageViewsSamplers.size(); ++texIdx)
	{
		imageInfos[texIdx].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[texIdx].imageView = textureImageViewsSamplers[texIdx].first;
		imageInfos[texIdx].sampler = textureImageViewsSamplers[texIdx].second;
	}

	for (size_t idx = 0; idx < m_Count; ++idx)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UBOs[idx]->GetVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_Size;

		std::vector<VkWriteDescriptorSet> descriptorWrites(textureImageViewsSamplers.size() + 1);

		// First write struct is for UBO
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_DescriptorSets[idx];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		for (size_t texIdx = 0; texIdx < textureImageViewsSamplers.size(); ++texIdx)
		{
			// Remaining write structs are for texture image view & sampler pairs
			descriptorWrites[1 + texIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1 + texIdx].dstSet = m_DescriptorSets[idx];
			descriptorWrites[1 + texIdx].dstBinding = 1 + static_cast<uint32_t>(texIdx);
			descriptorWrites[1 + texIdx].dstArrayElement = 0;
			descriptorWrites[1 + texIdx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1 + texIdx].descriptorCount = 1;
			descriptorWrites[1 + texIdx].pImageInfo = &imageInfos[texIdx];
		}

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(), 0, nullptr);
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t index)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&m_DescriptorSets[index], 0, nullptr);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSetLayout(const VulkanContext& context, size_t imageCount)
{
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings(imageCount + 1);

	layoutBindings[0].binding = 0; 
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	layoutBindings[0].descriptorCount = 1; 
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; 
	layoutBindings[0].pImmutableSamplers = nullptr; 

	for (int idx = 1; idx < layoutBindings.size() ; ++idx)
	{
		layoutBindings[idx].binding = idx; 
		layoutBindings[idx].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
		layoutBindings[idx].descriptorCount = 1; 
		layoutBindings[idx].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; 
		layoutBindings[idx].pImmutableSamplers = nullptr; 
	}

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size()); 
	layoutInfo.pBindings = layoutBindings.data(); 

	if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateUBOs(const VulkanContext& context)
{
	m_UBOs.resize(m_Count);
	m_UBOsMapped.resize(m_Count);
	
	for (int uboIndex = 0; uboIndex < m_Count; ++uboIndex)
	{
		m_UBOs[uboIndex] = new GP2_Buffer(context.device, context.physicalDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Size);
		m_UBOs[uboIndex]->Map(&m_UBOsMapped[uboIndex]);
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::SetUBO(UBO src, size_t index)
{
	memcpy(m_UBOsMapped[index], &src, m_Size); 
}

// The issue with your code is that you're creating a separate VkWriteDescriptorSet struct for each texture image view and sampler pair, 
// & calling vkUpdateDescriptorSets once for each pair. 
// This means that each texture image view and sampler pair is being written to a separate descriptor set, rather than being combined into a single descriptor set.

// In contrast, the working solution creates a single VkWriteDescriptorSet struct for all the texture image view and sampler pairs, 
// & calls vkUpdateDescriptorSets once for each descriptor set.
// This means that all the texture image view and sampler pairs are being combined into a single descriptor set, allowing the shader to access all the textures.