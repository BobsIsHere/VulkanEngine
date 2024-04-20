#pragma once
#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
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
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		//COLOR
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		//NORMAL
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);

		return attributeDescriptions;
	}
};

struct VertexUBO 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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