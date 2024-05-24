#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex2D
{
	glm::vec2 position;
	glm::vec3 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex2D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
		//POSITION
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, position);

		//COLOR
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, color);

		//TEXCOORD
		attributeDescriptions[2].binding = 0; 
		attributeDescriptions[2].location = 2; 
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT; 
		attributeDescriptions[2].offset = offsetof(Vertex2D, texCoord);

		return attributeDescriptions;
	}
};

struct Vertex3D
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal; 
	glm::vec3 tangent;

	static VkVertexInputBindingDescription GetBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{}; 
		bindingDescription.binding = 0; 
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 5> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
		//POSITION
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, position); 

		//COLOR
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, color); 

		//TEXCOORD
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex3D, texCoord); 

		//NORMAL
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex3D, normal);

		//TANGENT
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex3D, tangent);

		return attributeDescriptions;
	}
};

struct VertexUBO 
{
	//glm::mat4 model;
	glm::mat4 proj; 
	glm::mat4 view;
};

struct ViewProjection 
{
	glm::mat4 proj;
	glm::mat4 view;
};

struct MeshData 
{
	glm::mat4 model;
};