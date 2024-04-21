#pragma once
#include <vulkanbase/VulkanUtil.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>

#include "Vertex.h"

template<typename VertexType> 
class GP2_Shader final 
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile);
	~GP2_Shader() = default;

	//------------
	// Rule of 5
	//------------
	GP2_Shader(const GP2_Shader&) = delete;
	GP2_Shader(const GP2_Shader&&) = delete;
	GP2_Shader& operator=(const GP2_Shader&) = delete;
	GP2_Shader& operator=(const GP2_Shader&&) = delete;

	//-----------
	// Functions
	//-----------
	void Initialize(const VkDevice& vkDevice);

	void DestroyShaderModule(const VkDevice& vkDevice);

	std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() { return m_ShaderStages; };

	VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo(const VkDevice& vkDevice);
	VkPipelineShaderStageCreateInfo CreateVertexShaderInfo(const VkDevice& vkDevice);
	VkPipelineVertexInputStateCreateInfo CreateVertexInputStateInfo();
	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo();

private:
	//-----------
	// Functions
	//----------- 
	VkShaderModule CreateShaderModule(const VkDevice& vkDevice, const std::vector<char>& code);

	//-----------
	// Variables
	//-----------
	std::string m_VertexShaderFile;
	std::string m_FragmentShaderFile;

	VkPhysicalDevice m_PhysicalDevice;

	VkVertexInputBindingDescription m_BindingDescription;
	VkVertexInputAttributeDescription* m_AttributeDescriptions;   
	uint32_t m_AttributeCount;

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
};

template<typename VertexType>
GP2_Shader<VertexType>::GP2_Shader(const std::string& vertexShaderFile, const std::string& fragmentShaderFile) :
	m_VertexShaderFile{ vertexShaderFile },
	m_FragmentShaderFile{ fragmentShaderFile },
	m_PhysicalDevice{},
	m_AttributeCount{}
{
	m_BindingDescription = VertexType::GetBindingDescription(); 

	auto attributeDescriptions = VertexType::GetAttributeDescriptions(); 
	m_AttributeCount = static_cast<uint32_t>(attributeDescriptions.size()); 
	m_AttributeDescriptions = new VkVertexInputAttributeDescription[m_AttributeCount]; 
	std::copy(attributeDescriptions.begin(), attributeDescriptions.end(), m_AttributeDescriptions); 
}

template<typename VertexType>
void GP2_Shader<VertexType>::Initialize(const VkDevice& vkDevice)
{
	m_ShaderStages.push_back(CreateVertexShaderInfo(vkDevice));
	m_ShaderStages.push_back(CreateFragmentShaderInfo(vkDevice));
}

template<typename VertexType>
void GP2_Shader<VertexType>::DestroyShaderModule(const VkDevice& vkDevice)
{
	for (auto& stageInfo : m_ShaderStages)
	{
		vkDestroyShaderModule(vkDevice, stageInfo.module, nullptr);
	}

	m_ShaderStages.clear();
}

template<typename VertexType>
VkPipelineShaderStageCreateInfo GP2_Shader<VertexType>::CreateFragmentShaderInfo(const VkDevice& vkDevice)
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

template<typename VertexType>
VkPipelineShaderStageCreateInfo GP2_Shader<VertexType>::CreateVertexShaderInfo(const VkDevice& vkDevice)
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

template<typename VertexType>
VkPipelineVertexInputStateCreateInfo GP2_Shader<VertexType>::CreateVertexInputStateInfo()
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &m_BindingDescription;

	vertexInputInfo.vertexAttributeDescriptionCount = m_AttributeCount; 
	vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions;

	return vertexInputInfo;
}

template<typename VertexType>
VkPipelineInputAssemblyStateCreateInfo GP2_Shader<VertexType>::CreateInputAssemblyStateInfo()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	return inputAssembly;
}

template<typename VertexType>
VkShaderModule GP2_Shader<VertexType>::CreateShaderModule(const VkDevice& vkDevice, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule{};
	if (vkCreateShaderModule(vkDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
