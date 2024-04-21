#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Vertex.h"
#include "GP2_Shader.h"
#include "GP2_Buffer.h"
#include "vulkanbase/VulkanUtil.h"

template<typename VertexType> 
class GP2_Mesh final
{
public:
	GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice);
	~GP2_Mesh() = default;

	void Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void DestroyMesh();
	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);

	void AddVertex(const glm::vec3 pos, const glm::vec3 color);
	void AddVertices(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const glm::vec3 color);
	void AddIndices(const std::vector<uint16_t> indices); 

	bool ParseOBJ(const std::string& filename, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<uint16_t>& indices); 

private:
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	VkDevice m_Device; 
	VkPhysicalDevice m_PhysicalDevice; 

	GP2_Buffer* m_pVertexBuffer;
	GP2_Buffer* m_pIndexBuffer;
	
	std::vector<VertexType> m_MeshVertices;  
	std::vector<uint16_t> m_MeshIndices;

	MeshData m_VertexConstant; 
};

template<typename VertexType>
GP2_Mesh<VertexType>::GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice) :
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_VertexConstant{ glm::mat4(1.f) },
	m_pVertexBuffer{},
	m_pIndexBuffer{}
{
}

template<typename VertexType>
void GP2_Mesh<VertexType>::Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	//VERTEX BUFFER
	GP2_Buffer vertexStagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshVertices) * m_MeshVertices.size() };
	vertexStagingBuffer.TransferDeviceLocal(m_MeshVertices.data());

	m_pVertexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshVertices) * m_MeshVertices.size() };
	m_pVertexBuffer->CopyBuffer(vertexStagingBuffer, graphicsQueue, queueFamilyIndices);

	//INDEX BUFFER
	GP2_Buffer indexStagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshIndices) * m_MeshIndices.size() };
	indexStagingBuffer.TransferDeviceLocal(m_MeshVertices.data());

	m_pIndexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshIndices) * m_MeshIndices.size() };
	m_pIndexBuffer->TransferDeviceLocal(m_MeshIndices.data());
}

template<typename VertexType>
void GP2_Mesh<VertexType>::DestroyMesh()
{
	m_pIndexBuffer->Destroy();
	delete m_pIndexBuffer;

	m_pVertexBuffer->Destroy();
	delete m_pVertexBuffer;
}

template<typename VertexType>
void GP2_Mesh<VertexType>::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer)
{
	m_pVertexBuffer->BindAsVertexBuffer(buffer);
	m_pIndexBuffer->BindAsIndexBuffer(buffer);

	vkCmdPushConstants(
		buffer,
		pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT, // Stage flag should match the push constant range in the layout
		0,                          // Offset within the push constant block
		sizeof(MeshData),					// Size of the push constants to update
		&m_VertexConstant		   // Pointer to the data
	);

	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_MeshIndices.size()), 1, 0, 0, 0);
}

template<typename VertexType>
void GP2_Mesh<VertexType>::AddVertex(const glm::vec3 pos, const glm::vec3 color)
{
	m_MeshVertices.push_back(VertexType{ pos, color }); 
}

template<typename VertexType>
void GP2_Mesh<VertexType>::AddVertices(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const glm::vec3 color)
{
	for (uint64_t index = 0; index < vertices.size(); ++index)
	{
		m_MeshVertices.push_back(VertexType{ vertices[index], color, normals[index] });
	}
}

template<typename VertexType>
void GP2_Mesh<VertexType>::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices = indices;
}

template<typename VertexType>
bool GP2_Mesh<VertexType>::ParseOBJ(const std::string& filename, std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals, std::vector<uint16_t>& indices)
{
	std::ifstream file(filename);
	if (!file)
		return false;

	std::string sCommand;
	// start a while iteration ending when the end of file is reached (ios::eof)
	while (!file.eof())
	{
		//read the first word of the string, use the >> operator (istream::operator>>) 
		file >> sCommand;
		//use conditional statements to process the different commands	
		if (sCommand == "#")
		{
			// Ignore Comment
		}
		else if (sCommand == "v")
		{
			//Vertex
			float x, y, z;
			file >> x >> y >> z;
			positions.push_back({ x, y, z });
		}
		else if (sCommand == "f")
		{
			float i0, i1, i2;
			file >> i0 >> i1 >> i2;

			indices.push_back((int)i0 - 1);
			indices.push_back((int)i1 - 1);
			indices.push_back((int)i2 - 1);
		}
		//read till end of line and ignore all remaining chars
		file.ignore(1000, '\n');

		if (file.eof())
			break;
	}

	//Precompute normals
	for (uint64_t index = 0; index < indices.size(); index += 3)
	{
		uint32_t i0 = indices[index];
		uint32_t i1 = indices[index + 1];
		uint32_t i2 = indices[index + 2];

		glm::vec3 edgeV0V1 = positions[i1] - positions[i0];
		glm::vec3 edgeV0V2 = positions[i2] - positions[i0];
		glm::vec3 normal = CrossProduct(edgeV0V1, edgeV0V2);

		if (isnan(normal.x))
		{
			int k = 0;
		}

		Normalize(normal);
		if (isnan(normal.x))
		{
			int k = 0;
		}

		normals.push_back(normal);
	}

	return true;
}

template<typename VertexType>
uint32_t GP2_Mesh<VertexType>::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}