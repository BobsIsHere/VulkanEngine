#pragma once
#include <string>
#include <memory>
#include <vulkanbase/VulkanUtil.h>

#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_CommandBuffer.h"
#include "GP2_DescriptorPool.h"

class GP2_GraphicsPipeline
{
public:
	GP2_GraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);

	void Initialize(const VulkanContext& context);
	VkPushConstantRange CreatePushConstantRange();
	VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo();
	VkPipeline CreateGraphicsPipeline(const VulkanContext& context, uint32_t subpassIndex);

	void Cleanup(const VulkanContext& context);

	void Record(const GP2_CommandBuffer& buffer, VkExtent2D extent);
	void DrawScene(const GP2_CommandBuffer& buffer);
	void AddMesh(std::unique_ptr<GP2_Mesh> mesh);

	void SetUBO(ViewProjection ubo, size_t uboIndex);
private:
	// ...
	std::unique_ptr<GP2_DescriptorPool<ViewProjection>> m_UBOPool;
	std::unique_ptr <GP2_Shader> m_Shader;

	std::vector<GP2_Mesh> m_Meshes;

	VkRenderPass m_RenderPass;
};
