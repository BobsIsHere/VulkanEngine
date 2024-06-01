#pragma once
#include <string>
#include <memory>
#include <vulkanbase/VulkanUtil.h>
#include <vulkanbase/VulkanBase.h>

#include "Vertex.h"
#include "GP2_Mesh.h"
#include "GP2_Shader.h"
#include "GP2_Texture.h"
#include "GP2_CommandBuffer.h"
#include "GP2_DescriptorPool.h"

using pMesh3D = std::unique_ptr<GP2_Mesh<Vertex3D>>;

// Forward declaration of GP2_Texture because otherwise the compiler doesn't know it's a valid class
// I do not understand why this is necessary, Pieter-Jan does not know why this is needed either
// But I shall simply accept it
class GP2_Texture;

template<class UBOPBR>
class GP2_PBRMetallicPipeline
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_PBRMetallicPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_PBRMetallicPipeline() = default; 

	//-----------
	// Functions
	//-----------
	void Initialize(const VulkanContext& context);

	void SetTexturesMetallicPBR(const VulkanContext& context, VkQueue graphicsQueue, GP2_CommandPool commandPool, QueueFamilyIndices queueFamilyInd,
								const std::string albedo, const std::string normal, const std::string roughness, const std::string metallic);
	void CycleRenderingModes(); 

	void Cleanup();

	void Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx);
	void DrawScene(const GP2_CommandBuffer& buffer);
	void AddMesh(pMesh3D mesh);

	void SetUBO(UBOPBR ubo, size_t uboIndex);

private:
	//-----------
	// Functions
	//-----------
	void CreateGraphicsPipeline();
	std::vector<VkPushConstantRange> CreatePushConstantRange();

	//-----------
	// Variables
	//-----------
	VkDevice m_Device;
	VkRenderPass m_RenderPass;
	VkPipeline m_GraphicsPipeline;
	VkPipelineLayout m_PipelineLayout;

	GP2_Texture* m_DiffuseTexture;
	GP2_Texture* m_NormalTexture;
	GP2_Texture* m_RoughnessTexture;
	GP2_Texture* m_MetalnessTexture;

	GP2_Shader<Vertex3D> m_Shader;
	std::vector<pMesh3D> m_pMeshes;
	GP2_DescriptorPool<UBOPBR>* m_pDescriptorPool;

	RenderingModes m_RenderingMode;
};

template<class UBOPBR>
GP2_PBRMetallicPipeline<UBOPBR>::GP2_PBRMetallicPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_Device{},
	m_RenderPass{},
	m_GraphicsPipeline{},
	m_PipelineLayout{},
	m_Shader{ vertexShaderFile, fragmentShaderFile },
	m_pMeshes{},
	m_pDescriptorPool{},
	m_RenderingMode{ RenderingModes::Combined } 
{
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::Initialize(const VulkanContext& context)
{
	m_Device = context.device;
	m_RenderPass = context.renderPass;

	m_Shader.Initialize(m_Device);

	std::vector<std::pair<VkImageView, VkSampler>> textureImageViewsSamplers;

	textureImageViewsSamplers.push_back(std::make_pair(m_DiffuseTexture->GetTextureImageView(), m_DiffuseTexture->GetTextureSampler()));
	textureImageViewsSamplers.push_back(std::make_pair(m_NormalTexture->GetTextureImageView(), m_NormalTexture->GetTextureSampler()));
	textureImageViewsSamplers.push_back(std::make_pair(m_RoughnessTexture->GetTextureImageView(), m_RoughnessTexture->GetTextureSampler()));
	textureImageViewsSamplers.push_back(std::make_pair(m_MetalnessTexture->GetTextureImageView(), m_MetalnessTexture->GetTextureSampler()));

	m_pDescriptorPool = new GP2_DescriptorPool<UBOPBR>{ m_Device, MAX_FRAMES_IN_FLIGHT, textureImageViewsSamplers.size() };
	m_pDescriptorPool->Initialize(context, textureImageViewsSamplers);

	CreateGraphicsPipeline();
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::SetTexturesMetallicPBR(const VulkanContext& context, VkQueue graphicsQueue, GP2_CommandPool commandPool, QueueFamilyIndices queueFamilyInd,
															 const std::string albedo, const std::string normal, const std::string roughness, const std::string metallic)
{
	m_DiffuseTexture = new GP2_Texture{ context, graphicsQueue, commandPool }; 
	m_DiffuseTexture->Initialize(albedo.c_str(), VK_FORMAT_R8G8B8A8_SRGB, queueFamilyInd);

	m_NormalTexture = new GP2_Texture{ context, graphicsQueue, commandPool };
	m_NormalTexture->Initialize(normal.c_str(), VK_FORMAT_R8G8B8A8_UNORM, queueFamilyInd);

	m_RoughnessTexture = new GP2_Texture{ context, graphicsQueue, commandPool };
	m_RoughnessTexture->Initialize(roughness.c_str(), VK_FORMAT_R8G8B8A8_UNORM, queueFamilyInd);

	m_MetalnessTexture = new GP2_Texture{ context, graphicsQueue, commandPool };
	m_MetalnessTexture->Initialize(metallic.c_str(), VK_FORMAT_R8G8B8A8_UNORM, queueFamilyInd);
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::CycleRenderingModes()
{
	const int amountOfModes{ 4 }; 
	const int nextMode{ (static_cast<int>(m_RenderingMode) + 1) % amountOfModes };

	m_RenderingMode = static_cast<RenderingModes>(nextMode); 
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::Cleanup()
{
	for (size_t idx = 0; idx < m_pMeshes.size(); ++idx)
	{
		m_pMeshes[idx]->DestroyMesh();
	}

	vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);

	delete m_pDescriptorPool;

	m_DiffuseTexture->CleanUp();
	m_NormalTexture->CleanUp();
	m_RoughnessTexture->CleanUp();
	m_MetalnessTexture->CleanUp();

	delete m_DiffuseTexture;
	delete m_NormalTexture;
	delete m_RoughnessTexture;
	delete m_MetalnessTexture;
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::Record(const GP2_CommandBuffer& buffer, VkExtent2D extent, int imageIdx)
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

	vkCmdPushConstants(
		buffer.GetVkCommandBuffer(),
		m_PipelineLayout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(MeshData),
		sizeof(m_RenderingMode),
		&m_RenderingMode
	);

	DrawScene(buffer);
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::DrawScene(const GP2_CommandBuffer& buffer)
{
	m_pDescriptorPool->BindDescriptorSet(buffer.GetVkCommandBuffer(), m_PipelineLayout, 0);

	for (auto& mesh : m_pMeshes)
	{
		mesh->Draw(m_PipelineLayout, buffer.GetVkCommandBuffer());
	}
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::AddMesh(pMesh3D mesh)
{
	m_pMeshes.push_back(std::move(mesh));
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::SetUBO(UBOPBR ubo, size_t uboIndex)
{
	m_pDescriptorPool->SetUBO(ubo, uboIndex);
}

template<class UBOPBR>
void GP2_PBRMetallicPipeline<UBOPBR>::CreateGraphicsPipeline()
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
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

	std::vector<VkPushConstantRange> pushConstantRange = CreatePushConstantRange(); 
	pipelineLayoutInfo.pushConstantRangeCount = 2;
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.data(); 

	if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

	// depth of new fragments is compared to depth buffer to see if it should be discarded
	depthStencil.depthTestEnable = VK_TRUE;
	// new depth of fragments that pass test should be written to depth buffer
	depthStencil.depthWriteEnable = VK_TRUE;

	// comparison that is performed to keep or discard fragments
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	// allows to only keep fragments that fall within specified range
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;
	depthStencil.maxDepthBounds = 1.0f;

	// configure stencil buffer operations
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {};
	depthStencil.back = {};

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

	// depth stencil state
	pipelineInfo.pDepthStencilState = &depthStencil;
#pragma endregion pipelineInfo

	if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1,
		&pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	m_Shader.DestroyShaderModule(m_Device);
}

template<class UBOPBR>
std::vector<VkPushConstantRange> GP2_PBRMetallicPipeline<UBOPBR>::CreatePushConstantRange()
{
	std::vector<VkPushConstantRange> pushConstantRange(2); 

	pushConstantRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Stage the push constant is accessible from
	pushConstantRange[0].offset = 0;
	pushConstantRange[0].size = sizeof(MeshData); // Size of push constant block

	pushConstantRange[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Stage the push constant is accessible from 
	pushConstantRange[1].offset = sizeof(MeshData);
	pushConstantRange[1].size = sizeof(RenderingModes); // Size of push constant block

	return pushConstantRange;
}