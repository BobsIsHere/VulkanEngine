#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <array>

struct Vertex 
{
	glm::vec2 position;
	glm::vec3 color;

	glm::vec2 uv;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 viewDirection;

	static VkVertexInputBindingDescription GetBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
		//POSITION
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);
		//COLOR
		attributeDescriptions[1].binding = 0; 
		attributeDescriptions[1].location = 1; 
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; 
		attributeDescriptions[1].offset = offsetof(Vertex, color); 

		return attributeDescriptions;
	}
};

class GP2_Shader
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
	std::array<VkVertexInputAttributeDescription, 2Ui64> m_AttributeDescriptions;

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
};
