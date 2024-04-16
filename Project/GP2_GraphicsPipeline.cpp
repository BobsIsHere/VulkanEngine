#include "GP2_GraphicsPipeline.h"
#include "Vertex.h"
#include <vulkanbase/VulkanBase.h>

GP2_GraphicsPipeline::GP2_GraphicsPipeline(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_RenderPass{},
	m_UBOPool{},
	m_Meshes{}
{
	m_Shader = std::make_unique<GP2_Shader>("shaders/shader.vert.spv", "shaders/shader.frag.spv");
}

void GP2_GraphicsPipeline::Initialize(const VulkanContext& context)
{
	m_RenderPass = context.renderPass;
	m_Shader->Initialize(context.device);
	m_UBOPool = std::make_unique<GP2_DescriptorPool<ViewProjection>>(context.device, 1);
	m_UBOPool->Initialize(context);
	//CreateGraphicsPipeline(context); 
}

VkPushConstantRange GP2_GraphicsPipeline::CreatePushConstantRange()
{
	VkPushConstantRange pushConstantRange = {};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Stage the push constant is accessible from
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(MeshData); // Size of push constant block

	return pushConstantRange;
}

VkPipelineVertexInputStateCreateInfo GP2_GraphicsPipeline::CreateVertexInputStateInfo() 
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {}; 
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO; 

	// Get binding and attribute descriptions
	VkVertexInputBindingDescription bindingDescription = Vertex::GetBindingDescription();
	std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::GetAttributeDescriptions(); 

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()); 
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); 

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo GP2_GraphicsPipeline::CreateInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; 
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	return inputAssemblyInfo; 
}

VkPipeline GP2_GraphicsPipeline::CreateGraphicsPipeline(const VulkanContext& context, uint32_t subpassIndex)
{
   return VkPipeline();
}

void GP2_GraphicsPipeline::Cleanup(const VulkanContext& context)
{
}

void GP2_GraphicsPipeline::Record(const GP2_CommandBuffer& buffer, VkExtent2D extent)
{
}

void GP2_GraphicsPipeline::DrawScene(const GP2_CommandBuffer& buffer)
{
	/*m_UBOPool->BindDescriptorSet(buffer.GetVkCommandBuffer(), m_PipelineLayout, 0);

	for (GP2_Mesh& mesh : m_Meshes) 
	{
		mesh.Draw(m_PipelineLayout, buffer.GetVkCommandBuffer());
	}*/
}

void GP2_GraphicsPipeline::AddMesh(std::unique_ptr<GP2_Mesh> mesh)
{
}

void GP2_GraphicsPipeline::SetUBO(ViewProjection ubo, size_t uboIndex)
{

}
