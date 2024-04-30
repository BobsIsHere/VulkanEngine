#pragma once
#include <string>
#include <memory>
#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

#include "Vertex.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_CommandBuffer.h"
#include "GP2_DescriptorPool.h"

using pMesh2D = std::unique_ptr<GP2_Mesh<Vertex2D>>;  

template <class UBO2D>
class GP2_2DGraphicsPipeline final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_2DGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_2DGraphicsPipeline() = default;

	//-----------
	// Functions
	//-----------
	void Initialize(const VulkanContext& context, VkImageView textureImageView, VkSampler textureSampler);

	void Cleanup();

	void Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx);
	void DrawScene(const GP2_CommandBuffer& buffer);
	void AddMesh(pMesh2D mesh); 
	 
	void SetUBO(UBO2D ubo, size_t uboIndex);

private:
	//-----------
	// Functions
	//-----------
	void CreateGraphicsPipeline(); 
	VkPushConstantRange CreatePushConstantRange();

	//-----------
	// Variables
	//-----------
	VkDevice m_Device;
	VkRenderPass m_RenderPass;
	VkPipeline m_GraphicsPipeline;
	VkPipelineLayout m_PipelineLayout;

	GP2_Shader<Vertex2D> m_Shader;  
	std::vector<pMesh2D> m_pMeshes; 
	GP2_DescriptorPool<UBO2D>* m_pDescriptorPool;
};

template <class UBO2D>
GP2_2DGraphicsPipeline<UBO2D>::GP2_2DGraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_Device{},
	m_RenderPass{},
	m_GraphicsPipeline{},
	m_PipelineLayout{},
	m_Shader{ vertexShaderFile, fragmentShaderFile },
	m_pMeshes{},
	m_pDescriptorPool{}
{
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::Initialize(const VulkanContext& context, VkImageView textureImageView, VkSampler textureSampler)
{
	m_Device = context.device;
	m_RenderPass = context.renderPass;

	m_Shader.Initialize(m_Device);

	m_pDescriptorPool = new GP2_DescriptorPool<UBO2D>{ m_Device, MAX_FRAMES_IN_FLIGHT };
	m_pDescriptorPool->Initialize(context, textureImageView, textureSampler);

	CreateGraphicsPipeline();
}

template <class UBO2D>
VkPushConstantRange GP2_2DGraphicsPipeline<UBO2D>::CreatePushConstantRange()
{
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Stage the push constant is accessible from
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(MeshData); // Size of push constant block

	return pushConstantRange;
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::CreateGraphicsPipeline()
{
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_pDescriptorPool->GetDescriptorSetLayout();
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VkPushConstantRange pushConstantRange = CreatePushConstantRange();
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};

#pragma region pipelineInfo 
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = m_Shader.GetShaderStages().data();

	pipelineInfo.pVertexInputState = &m_Shader.CreateVertexInputStateInfo(); 
	pipelineInfo.pInputAssemblyState = &m_Shader.CreateInputAssemblyStateInfo();

	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.renderPass = m_RenderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
#pragma endregion pipelineInfo

	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1,
		&pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	m_Shader.DestroyShaderModule(m_Device);
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::Cleanup()
{
	for (size_t idx = 0; idx < m_pMeshes.size(); ++idx)
	{
		m_pMeshes[idx]->DestroyMesh();
	}

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

	delete m_pDescriptorPool;
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx)
{
	vkCmdBindPipeline(buffer.GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(buffer.GetVkCommandBuffer(), 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetScissor(buffer.GetVkCommandBuffer(), 0, 1, &scissor);

	m_pDescriptorPool->BindDescriptorSet(buffer.GetVkCommandBuffer(), m_PipelineLayout, imageIdx);

	DrawScene(buffer);
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::DrawScene(const GP2_CommandBuffer& buffer)
{
	m_pDescriptorPool->BindDescriptorSet(buffer.GetVkCommandBuffer(), m_PipelineLayout, 0); 

	for (auto& mesh : m_pMeshes)
	{
		mesh->Draw(m_PipelineLayout, buffer.GetVkCommandBuffer());
	}
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::AddMesh(pMesh2D mesh)  
{
	m_pMeshes.push_back(std::move(mesh));
}

template <class UBO2D>
void GP2_2DGraphicsPipeline<UBO2D>::SetUBO(UBO2D ubo, size_t uboIndex)
{
	m_pDescriptorPool->SetUBO(ubo, uboIndex);
}