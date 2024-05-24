#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>

#include "Vertex.h"
#include "GP2_Shader.h"
#include "GP2_Buffer.h"
#include "GP2_Texture.h"
#include "vulkanbase/VulkanUtil.h"

template<typename VertexType> 
class GP2_Mesh final
{
public:
	//---------------------------
	// Constructors & Destructor
	//---------------------------
	GP2_Mesh(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool);
	~GP2_Mesh() = default;

	//-----------
	// Functions
	//-----------
	void Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices);
	void DestroyMesh();
	void Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer);
	
	void AddVertices(std::vector<VertexType> vertices);
	void AddIndices(const std::vector<uint16_t> indices); 
	void AddTexture(const char* filePath);

	const char* GetTexture(size_t index) const { return m_pTextures[index]; }

	bool ParseOBJ(const std::string& filename, const glm::vec3 color); 

private:
	//-----------
	// Functions
	//-----------
	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

	//-----------
	// Variables
	//-----------
	std::vector<const char*> m_pTextures;

	VkDevice m_Device; 
	VkPhysicalDevice m_PhysicalDevice; 
	VkQueue m_GraphicsQueue;

	VulkanContext m_Context;
	GP2_CommandPool m_CommandPool;

	GP2_Buffer* m_pVertexBuffer;
	GP2_Buffer* m_pIndexBuffer;
	
	std::vector<VertexType> m_MeshVertices;  
	std::vector<uint16_t> m_MeshIndices;

	MeshData m_VertexConstant; 
};

template<typename VertexType>
GP2_Mesh<VertexType>::GP2_Mesh(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool) :
	m_Context{ context },
	m_Device{ context.device },
	m_PhysicalDevice{ context.physicalDevice },
	m_GraphicsQueue{ graphicsQueue },
	m_CommandPool{ commandPool },
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
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshVertices[0])* m_MeshVertices.size()};
	vertexStagingBuffer.TransferDeviceLocal(m_MeshVertices.data());

	m_pVertexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(m_MeshVertices[0])* m_MeshVertices.size()};
	m_pVertexBuffer->CopyBuffer(vertexStagingBuffer, graphicsQueue, queueFamilyIndices);

	//INDEX BUFFER
	GP2_Buffer indexStagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshIndices[0])* m_MeshIndices.size()};
	indexStagingBuffer.TransferDeviceLocal(m_MeshIndices.data());

	m_pIndexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(m_MeshIndices[0])* m_MeshIndices.size()};
	m_pIndexBuffer->CopyBuffer(indexStagingBuffer, graphicsQueue, queueFamilyIndices);

	vertexStagingBuffer.Destroy();
	indexStagingBuffer.Destroy();
}

template<typename VertexType>
void GP2_Mesh<VertexType>::DestroyMesh()
{
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Destroy();
		delete m_pIndexBuffer;
		m_pIndexBuffer = nullptr;
	}

	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Destroy();
		delete m_pVertexBuffer;
		m_pVertexBuffer = nullptr;
	}
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
inline void GP2_Mesh<VertexType>::AddVertices(std::vector<VertexType> vertices) 
{
	m_MeshVertices.insert(m_MeshVertices.end(), vertices.begin(), vertices.end());
}

template<typename VertexType>
void GP2_Mesh<VertexType>::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices.insert(m_MeshIndices.end(), indices.begin(), indices.end());
}

template<typename VertexType>
inline void GP2_Mesh<VertexType>::AddTexture(const char* filePath)
{
	m_pTextures.push_back(filePath); 
}

template<typename VertexType>
bool GP2_Mesh<VertexType>::ParseOBJ(const std::string& filename, const glm::vec3 color)
{
	std::ifstream file{ filename };
	if (!file.is_open()) 
	{
		std::cerr << "Error: Failed to open file " << filename << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions; 
	std::vector<glm::vec3> normals; 
	std::vector<glm::vec2> texCoordinates;

	Vertex3D vertex{};

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

			positions.emplace_back(x, y, z);
		}
		else if (sCommand == "vn")
		{
			// Vertex Normal
			float x, y, z;
			file >> x >> y >> z;

			normals.emplace_back(x, y, z);
		}
		else if (sCommand == "vt")
		{
			// Vertex Texture Coordinate
			float u, v; 
			file >> u >> v; 

			texCoordinates.emplace_back(u, v);
		}
		else if (sCommand == "f")
		{
			//if a face is read:
			//construct the 3 vertices, add them to the vertex array
			//add three indices to the index array
			//add the material index as attibute to the attribute array

			// Faces or triangles
			size_t iPosition, iNormal, iTexCoord;  

			uint32_t tempIndices[3];
			for (size_t iFace = 0; iFace < 3; ++iFace)
			{	
				// OBJ format uses 1-based arrays
				file >> iPosition;
				vertex.position = glm::vec3(positions[iPosition - 1].x, -positions[iPosition - 1].y, -positions[iPosition - 1].z); 
				vertex.color = color;

				if ('/' == file.peek())//is next in buffer ==  '/' ?
				{
					file.ignore();//read and ignore one element ('/')

					if ('/' != file.peek()) 
					{
						// Optional texture coordinate
						file >> iTexCoord; 
						vertex.texCoord = glm::vec2(texCoordinates[iTexCoord - 1].x, -texCoordinates[iTexCoord - 1].y);
					}

					if ('/' == file.peek())
					{
						file.ignore();

						// Optional vertex normal
						file >> iNormal;
						vertex.normal = glm::vec3(normals[iNormal - 1].x, -normals[iNormal - 1].y, -normals[iNormal - 1].z); 
					}
				}

				m_MeshVertices.push_back(vertex);
				tempIndices[iFace] = uint32_t(m_MeshVertices.size()) - 1;  
			}

			m_MeshIndices.push_back(tempIndices[0]);
			m_MeshIndices.push_back(tempIndices[1]);
			m_MeshIndices.push_back(tempIndices[2]);
		}

		//read till end of line and ignore all remaining chars
		file.ignore(1000, '\n');
	}

	file.close(); 
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