#include "GP2_2DMesh.h"

GP2_2DMesh::GP2_2DMesh(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool) :
	m_Device{ context.device },
	m_PhysicalDevice{ context.physicalDevice },
	m_VertexConstant{ glm::mat4(1.f) },
	m_pVertexBuffer{},
	m_pIndexBuffer{},
	//m_pTextures(5)
{
	/*for (auto& pTexture : m_pTextures)
	{
		pTexture = new GP2_Texture{ context, graphicsQueue, commandPool };
	}*/
}

void GP2_2DMesh::Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	//VERTEX BUFFER
	GP2_Buffer vertexStagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshVertices[0]) * m_MeshVertices.size() };
	vertexStagingBuffer.TransferDeviceLocal(m_MeshVertices.data());

	m_pVertexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(m_MeshVertices[0]) * m_MeshVertices.size() };
	m_pVertexBuffer->CopyBuffer(vertexStagingBuffer, graphicsQueue, queueFamilyIndices);

	//INDEX BUFFER
	GP2_Buffer indexStagingBuffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MeshIndices[0]) * m_MeshIndices.size() };
	indexStagingBuffer.TransferDeviceLocal(m_MeshIndices.data());

	m_pIndexBuffer = new GP2_Buffer{ m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sizeof(m_MeshIndices[0]) * m_MeshIndices.size() };
	m_pIndexBuffer->CopyBuffer(indexStagingBuffer, graphicsQueue, queueFamilyIndices);

	/*for (const auto& pTexture : m_pTextures)
	{
		pTexture->CreateTextureImage("resources/texture.jpg");
		pTexture->CreateTextureImageView();
		pTexture->CreateTextureSampler();
	}*/

	vertexStagingBuffer.Destroy();
	indexStagingBuffer.Destroy();
}

void GP2_2DMesh::DestroyMesh()
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

	/*for (auto pTexture : m_pTextures)
	{
		delete pTexture;
	}*/
}

void GP2_2DMesh::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer)
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

void GP2_2DMesh::AddVertex(const glm::vec3 pos, const glm::vec3 color)
{
	m_MeshVertices.push_back(Vertex2D{ pos, color });
}

void GP2_2DMesh::AddVertex(const glm::vec3 pos, const glm::vec3 color, const glm::vec2 texCoord)
{
	m_MeshVertices.push_back(Vertex2D{ pos, color, texCoord });
}

void GP2_2DMesh::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices = indices;
}

//GP2_Texture* GP2_2DMesh::GetTexture(const int index) const
//{
//	return m_pTextures[index];
//}

uint32_t GP2_2DMesh::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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