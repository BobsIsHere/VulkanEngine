#pragma once
#include "vulkanbase/VulkanUtil.h"
#include "GP2_Buffer.h"
#include "Vertex.h"

template <class VertexUBO>
class GP2_UniformBufferObject
{
public:
	void Initialize(const VulkanContext& context);
	void Upload();
	void SetData(VertexUBO ubo)
	{
		m_UBOSrc = ubo;
	}

	VkBuffer GetVkBuffer()
	{
		return m_UBOBuffer->GetVkBuffer();
	}

private:
	std::unique_ptr<GP2_Buffer> m_UBOBuffer;
	VertexUBO m_UBOSrc{};
};

template<class VertexUBO>
inline void GP2_UniformBufferObject<VertexUBO>::Initialize(const VulkanContext& context)
{
	m_UBOBuffer = std::make_unique<GP2_Buffer>( 
		context.device,
		context.physicalDevice,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(VertexUBO) 
	);
}

template<class VertexUBO>
inline void GP2_UniformBufferObject<VertexUBO>::Upload()
{
	m_UBOBuffer->Upload(sizeof(VertexUBO), &m_UBOSrc); 
}

template<class VertexUBO>
using GP2_UniformBufferObjectPtr = std::unique_ptr<GP2_UniformBufferObject<VertexUBO>>; 