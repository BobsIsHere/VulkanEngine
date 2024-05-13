#include "GP2_3DMesh.h"

GP2_3DMesh::GP2_3DMesh(VulkanContext context, VkQueue graphicsQueue, GP2_CommandPool commandPool) :
	m_Device{ context.device },
	m_PhysicalDevice{ context.physicalDevice },
	m_VertexConstant{ glm::mat4(1.f) },
	m_pVertexBuffer{},
	m_pIndexBuffer{},
	m_pTextures(5)
{
	for (auto& pTexture : m_pTextures)
	{
		pTexture = new GP2_Texture{ context, graphicsQueue, commandPool };
	}
}

void GP2_3DMesh::Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
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

	for (const auto& pTexture : m_pTextures)
	{
		pTexture->CreateTextureImage("resources/texture.jpg");
		pTexture->CreateTextureImageView();
		pTexture->CreateTextureSampler();
	}

	vertexStagingBuffer.Destroy();
	indexStagingBuffer.Destroy();
}

void GP2_3DMesh::DestroyMesh()
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

	for (auto pTexture : m_pTextures)
	{
		delete pTexture;
	}
}

void GP2_3DMesh::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer)
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

void GP2_3DMesh::AddVertex(const glm::vec3 pos, const glm::vec3 color)
{
	m_MeshVertices.push_back(Vertex3D{ pos, color });
}

void GP2_3DMesh::AddVertex(const glm::vec3 pos, const glm::vec3 color, const glm::vec3 normal, const glm::vec2 texCoord)
{
	m_MeshVertices.push_back(Vertex3D{ pos, color, normal, texCoord });
}

void GP2_3DMesh::AddVertices(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const glm::vec3 color)
{
	for (uint64_t index = 0; index < vertices.size(); ++index)
	{
		m_MeshVertices.push_back(Vertex3D{ vertices[index], color, normals[index] });
	}
}

void GP2_3DMesh::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices = indices; 
}

bool GP2_3DMesh::ParseOBJ(const std::string& filename, const glm::vec3 color)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "Error: Failed to open file " << filename << std::endl;
		return false;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;

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
		else if (sCommand == "f")
		{
			//if a face is read:
			//construct the 3 vertices, add them to the vertex array
			//add three indices to the index array
			//add the material index as attibute to the attribute array

			// Faces or triangles
			Vertex3D vertex{};
			size_t iPosition, iNormal;

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
						/*file >> iTexCoord;
						vertex.uv = UVs[iTexCoord - 1];*/
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
				//indices.push_back(uint32_t(vertices.size()) - 1);
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

uint32_t GP2_3DMesh::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
