#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>

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
	std::array<VkVertexInputAttributeDescription, 3> m_AttributeDescriptions;

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
};
