#include <vulkanbase/VulkanUtil.h>
#include <stdexcept>
#include "GP2_Shader.h"

GP2_Shader::GP2_Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_VertexShaderFile{ vertexShaderFile },
	m_FragmentShaderFile{ fragmentShaderFile }
	/*m_VertexBuffer{},
	m_VertexBufferMemory{}*/
{
	m_PhysicalDevice = VK_NULL_HANDLE;

	m_BindingDescription = Vertex::GetBindingDescription();
	m_AttributeDescriptions = Vertex::GetAttributeDescriptions(); 
}

void GP2_Shader::Initialize(const VkDevice& vkDevice)
{
	m_ShaderStages.push_back(CreateVertexShaderInfo(vkDevice));
	m_ShaderStages.push_back(CreateFragmentShaderInfo(vkDevice));
}

//void GP2_Shader::CreateVertexBuffer(const VkDevice& vkDevice)
//{
//	VkBufferCreateInfo bufferInfo{};
//	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//	bufferInfo.size = sizeof(m_Vertices[0]) * m_Vertices.size();
//	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//
//	if (vkCreateBuffer(vkDevice, &bufferInfo, nullptr, &m_VertexBuffer) != VK_SUCCESS)
//	{
//		throw std::runtime_error("failed to create vertex buffer!");
//	}
//
//	VkMemoryRequirements memRequirements; 
//	vkGetBufferMemoryRequirements(vkDevice, m_VertexBuffer, &memRequirements);
//
//	VkMemoryAllocateInfo allocInfo{}; 
//	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; 
//	allocInfo.allocationSize = memRequirements.size; 
//	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
//
//	if (vkAllocateMemory(vkDevice, &allocInfo, nullptr, &m_VertexBufferMemory) != VK_SUCCESS) 
//	{
//		throw std::runtime_error("failed to allocate vertex buffer memory!");
//	}
//
//	vkBindBufferMemory(vkDevice, m_VertexBuffer, m_VertexBufferMemory, 0); 
//
//	void* data{};
//	vkMapMemory(vkDevice, m_VertexBufferMemory, 0, bufferInfo.size, 0, &data);
//		memcpy(data, m_Vertices.data(), (size_t)bufferInfo.size);
//	vkUnmapMemory(vkDevice, m_VertexBufferMemory); 
//}

std::vector<Vertex> GP2_Shader::GetVertices() const
{
	return m_Vertices;
}

//VkBuffer GP2_Shader::GetVertexBuffer() const
//{
//	return m_VertexBuffer;
//}

void GP2_Shader::DestroyShaderModule(const VkDevice& vkDevice)
{
	for (auto& stageInfo : m_ShaderStages)
	{
		vkDestroyShaderModule(vkDevice, stageInfo.module, nullptr);
	}

	m_ShaderStages.clear();
}

//void GP2_Shader::DestroyVertexBuffer(const VkDevice& vkDevice)
//{
//	vkDestroyBuffer(vkDevice, m_VertexBuffer, nullptr);
//	vkFreeMemory(vkDevice, m_VertexBufferMemory, nullptr);
//}

//uint32_t GP2_Shader::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
//{
//	VkPhysicalDeviceMemoryProperties memProperties; 
//	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);
//
//	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
//	{
//		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
//		{
//			return i;
//		}
//	}
//
//	throw std::runtime_error("failed to find suitable memory type!");
//}

VkPipelineShaderStageCreateInfo GP2_Shader::CreateFragmentShaderInfo(const VkDevice& vkDevice)
{
	std::vector<char> fragShaderCode = readFile(m_FragmentShaderFile);
	VkShaderModule fragShaderModule = CreateShaderModule(vkDevice, fragShaderCode);

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	return fragShaderStageInfo;
}

VkPipelineShaderStageCreateInfo GP2_Shader::CreateVertexShaderInfo(const VkDevice& vkDevice)
{
	std::vector<char> vertShaderCode = readFile(m_VertexShaderFile);
	VkShaderModule vertShaderModule = CreateShaderModule(vkDevice, vertShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	return vertShaderStageInfo;
}

VkPipelineVertexInputStateCreateInfo GP2_Shader::CreateVertexInputStateInfo()
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_AttributeDescriptions.size()); 
	vertexInputInfo.pVertexBindingDescriptions = &m_BindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo GP2_Shader::CreateInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

VkShaderModule GP2_Shader::CreateShaderModule(const VkDevice& vkDevice, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}