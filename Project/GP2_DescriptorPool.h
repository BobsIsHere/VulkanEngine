#pragma once
#include <stdexcept>
#include <vector>
#include <memory>
#include "Vertex.h"
#include "vulkan/vulkan_core.h"
#include "GP2_UniformBufferObject.h"

template <class VertexUBO> 
class GP2_DescriptorPool
{
public:
	GP2_DescriptorPool(VkDevice device, size_t count);
	~GP2_DescriptorPool();

	void Initialize(const VulkanContext& context);

	void SetUBO(VertexUBO data, size_t index);

	const VkDescriptorSetLayout& GetDescriptorSetLayout()
	{
		return m_DescriptorSetLayout;
	}

	void CreateDescriptorSets();

	void BindDescriptorSet(VkCommandBuffer buffer, VkPipelineLayout layout, size_t index);

private:
	VkDevice m_Device;
	VkDeviceSize m_Size;
	VkDescriptorSetLayout m_DescriptorSetLayout;

	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSet> m_DescriptorSets;
	std::vector<GP2_UniformBufferObjectPtr<VertexUBO>> m_UBOs;

	size_t m_Count;

	void CreateDescriptorSetLayout(const VulkanContext& context);
	void CreateUBOs(const VulkanContext& context);
};

template <class VertexUBO>
GP2_DescriptorPool<VertexUBO>::GP2_DescriptorPool(VkDevice device, size_t count) :
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

template <class VertexUBO>
GP2_DescriptorPool<VertexUBO>::~GP2_DescriptorPool()
{
	for (GP2_UniformBufferObjectPtr<VertexUBO>& buffer : m_UBOs) 
	{
		buffer.reset();
	}
	vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
}

template<class VertexUBO>
inline void GP2_DescriptorPool<VertexUBO>::Initialize(const VulkanContext& context)
{
	CreateDescriptorSetLayout(context); 
	CreateUBOs(context); 
	CreateDescriptorSets(); 
}

template <class VertexUBO>
void GP2_DescriptorPool<VertexUBO>::CreateDescriptorSets()
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

	size_t descriptorIndex = 0;
	for (GP2_UniformBufferObjectPtr<VertexUBO>& buffer : m_UBOs) 
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = buffer->GetVkBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_Size;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[descriptorIndex];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(m_Device, 1, &descriptorWrite, 0, nullptr);
		++descriptorIndex;
	}
}

template <class VertexUBO>
void GP2_DescriptorPool<VertexUBO>::BindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, size_t index)
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, 
				&m_DescriptorSets[index], 0, nullptr);
}

template<class VertexUBO>
inline void GP2_DescriptorPool<VertexUBO>::CreateDescriptorSetLayout(const VulkanContext& context)
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

template<class VertexUBO>
inline void GP2_DescriptorPool<VertexUBO>::CreateUBOs(const VulkanContext& context)
{
	for (int uboIndex = 0; uboIndex < m_Count; ++uboIndex)
	{
		GP2_UniformBufferObjectPtr<VertexUBO> buffer = std::make_unique<GP2_UniformBufferObject<VertexUBO>>(); 
		buffer->Initialize(context);
		m_UBOs.emplace_back(std::move(buffer));
	}
}

template<class VertexUBO>
inline void GP2_DescriptorPool<VertexUBO>::SetUBO(VertexUBO src, size_t index)
{
	if (index < m_UBOs.size())
	{
		m_UBOs[index]->setData(src);
		m_UBOs[index]->upload();
	}
}