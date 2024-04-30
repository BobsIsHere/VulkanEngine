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
	GP2_DescriptorPool(VkDevice device, size_t count);
	~GP2_DescriptorPool();

	//-----------
	// Functions
	//-----------
	void Initialize(const VulkanContext& context, VkImageView textureImageView, VkSampler textureSampler); 

	void SetUBO(UBO data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout()
	{
		return m_DescriptorSetLayout;
	}

	void CreateDescriptorSets(VkImageView textureImageView, VkSampler textureSampler);

	void BindDescriptorSet(VkCommandBuffer buffer, VkPipelineLayout layout, size_t index);

private:
	//-----------
	// Functions
	//-----------
	void CreateDescriptorSetLayout(const VulkanContext& context);
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
GP2_DescriptorPool<UBO>::GP2_DescriptorPool(VkDevice device, size_t count) :
	m_Device{ device },
	m_Size{ sizeof(VertexUBO) },
	m_Count(count),
	m_DescriptorPool{ nullptr },
	m_DescriptorSetLayout{ nullptr }
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); 

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
inline void GP2_DescriptorPool<UBO>::Initialize(const VulkanContext& context, VkImageView textureImageView, VkSampler textureSampler)
{
	CreateDescriptorSetLayout(context);
	CreateUBOs(context);
	CreateDescriptorSets(textureImageView, textureSampler);
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSets(VkImageView textureImageView, VkSampler textureSampler)
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
	
	for (size_t idx = 0; idx < m_Count; ++idx) 
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_UBOs[idx]->GetVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_Size;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView; 
		imageInfo.sampler = textureSampler; 

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
		descriptorWrites[0].dstSet = m_DescriptorSets[idx];  
		descriptorWrites[0].dstBinding = 0; 
		descriptorWrites[0].dstArrayElement = 0; 
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
		descriptorWrites[0].descriptorCount = 1; 
		descriptorWrites[0].pBufferInfo = &bufferInfo; 

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET; 
		descriptorWrites[1].dstSet = m_DescriptorSets[idx];
		descriptorWrites[1].dstBinding = 1; 
		descriptorWrites[1].dstArrayElement = 0; 
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
		descriptorWrites[1].descriptorCount = 1; 
		descriptorWrites[1].pImageInfo = &imageInfo; 

		vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr); 
	}
}

template<class UBO>
void GP2_DescriptorPool<UBO>::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t index)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
		&m_DescriptorSets[index], 0, nullptr);
}

template<class UBO>
inline void GP2_DescriptorPool<UBO>::CreateDescriptorSetLayout(const VulkanContext& context)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{}; 
	uboLayoutBinding.binding = 0; 
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 
	uboLayoutBinding.descriptorCount = 1; 
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; 
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{}; 
	samplerLayoutBinding.binding = 1; 
	samplerLayoutBinding.descriptorCount = 1; 
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 
	samplerLayoutBinding.pImmutableSamplers = nullptr; 
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding }; 
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size()); 
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(context.device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

template<class UBO>
inline void GP2_DescriptorPool<UBO>::CreateUBOs(const VulkanContext& context)
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
inline void GP2_DescriptorPool<UBO>::SetUBO(UBO src, size_t index)
{
	memcpy(m_UBOsMapped[index], &src, m_Size); 
}