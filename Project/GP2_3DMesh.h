#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <glm/glm.hpp>

#include "Vertex.h"
#include "GP2_Shader.h"
#include "GP2_Buffer.h"
#include "GP2_Texture.h"
#include "GP2_CommandPool.h"
#include "vulkanbase/VulkanUtil.h"

class GP2_3DMesh final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_3DMesh(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool);
	~GP2_3DMesh() = default;

	//-----------
	// Functions
	//-----------
	void Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void DestroyMesh();
	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);

	void AddVertex(const glm::vec3 pos, const glm::vec3 color);
	void AddVertex(const glm::vec3 pos, const glm::vec3 color, const glm::vec3 normal, const glm::vec2 texCoord);
	void AddVertices(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const glm::vec3 color);
	void AddIndices(const std::vector<uint16_t> indices);

	GP2_Texture* GetTexture(const int index) const { return m_pTextures[index]; }

	bool ParseOBJ(const std::string& filename, const glm::vec3 color);

private:
	//-----------
	// Functions
	//-----------
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//-----------
	// Variables
	//-----------
	VkDevice m_Device;
	VkPhysicalDevice m_PhysicalDevice;

	GP2_Buffer* m_pVertexBuffer;
	GP2_Buffer* m_pIndexBuffer;

	std::vector<Vertex3D> m_MeshVertices;  
	std::vector<uint16_t> m_MeshIndices;
	std::vector<GP2_Texture*> m_pTextures;

	MeshData m_VertexConstant;
};
