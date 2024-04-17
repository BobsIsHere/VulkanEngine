#pragma once
#include <stdexcept>
#include <vector>
#include "Vertex.h"
#include "GP2_Buffer.h"
#include "vulkan/vulkan_core.h"
#include "vulkanbase/VulkanUtil.h"

template <class UBO>
class GP2_DescriptorPool final
{
public:
	GP2_DescriptorPool(VkDevice device, size_t count);
	~GP2_DescriptorPool();

	void Initialize(const VulkanContext& context);

	void SetUBO(UBO data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout()
	{
		return m_DescriptorSetLayout;
	}

	void CreateDescriptorSets();

	void BindDescriptorSet(VkCommandBuffer buffer, VkPipelineLayout layout, size_t index);

private:
	void CreateDescriptorSetLayout(const VulkanContext& context);
	void CreateUBOs(const VulkanContext& context);

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
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(count);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(count);

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
inline void GP2_DescriptorPool<UBO>::Initialize(const VulkanContext& context)
{
	CreateDescriptorSetLayout(context);
	CreateUBOs(context);
	CreateDescriptorSets();
}

template<class UBO>
void GP2_DescriptorPool<UBO>::CreateDescriptorSets()
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

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[idx]; 
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr; 

		vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
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

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

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
	memcpy(m_UBOs[index], &src, m_Size); 
}