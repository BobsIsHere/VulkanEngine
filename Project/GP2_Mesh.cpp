#include "GP2_Mesh.h"
#include "vulkanbase/VulkanBase.h"

GP2_Mesh::GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice) :
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_pBuffer{}
{
	m_pBuffer = std::make_unique<GP2_Buffer>(device, physicalDevice);
}

void GP2_Mesh::Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	m_pBuffer->CreateVertexBuffer(m_MeshVertices, graphicsQueue, queueFamilyIndices);
	m_pBuffer->CreateIndexBuffer(m_MeshIndices, graphicsQueue, queueFamilyIndices);
}

void GP2_Mesh::DestroyMesh()
{
	m_pBuffer->DestroyVertexBuffer();
	m_pBuffer->DestroyIndexBuffer();
}

void GP2_Mesh::Draw(VkCommandBuffer buffer)
{
	VkBuffer vertexBuffers[] = { m_pBuffer->GetVertexBuffer() }; 
	VkDeviceSize offsets[] = { 0 };
	m_pBuffer->BindBuffers(buffer, vertexBuffers, offsets); 
	
	vkCmdDrawIndexed(buffer, static_cast<uint32_t>(m_MeshIndices.size()), 1, 0, 0, 0);
}

void GP2_Mesh::AddVertex(const glm::vec2 pos, const glm::vec3 color)
{
	m_MeshVertices.push_back(Vertex{ pos, color }); 
}

void GP2_Mesh::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices = indices;
}

bool GP2_Mesh::ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint16_t>& indices, bool flipAxisAndWinding)
{
	std::ifstream file(filename);
	if (!file)
		return false;

	std::vector<glm::vec3> positions{}; 
	std::vector<glm::vec3> normals{}; 
	std::vector<glm::vec2> UVs{};

	vertices.clear();
	indices.clear();

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
		else if (sCommand == "vt")
		{
			// Vertex TexCoord
			float u, v;
			file >> u >> v;
			UVs.emplace_back(u, 1 - v);
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
			//
			// Faces or triangles
			Vertex vertex{};
			size_t iPosition, iTexCoord, iNormal;

			uint32_t tempIndices[3];
			for (size_t iFace = 0; iFace < 3; iFace++)
			{
				// OBJ format uses 1-based arrays
				file >> iPosition;
				vertex.position = positions[iPosition - 1];

				if ('/' == file.peek())//is next in buffer ==  '/' ?
				{
					file.ignore();//read and ignore one element ('/')

					if ('/' != file.peek())
					{
						// Optional texture coordinate
						file >> iTexCoord;
						vertex.uv = UVs[iTexCoord - 1];
					}

					if ('/' == file.peek())
					{
						file.ignore();

						// Optional vertex normal
						file >> iNormal;
						vertex.normal = normals[iNormal - 1];
					}
				}

				vertices.push_back(vertex);
				tempIndices[iFace] = uint32_t(vertices.size()) - 1;
				//indices.push_back(uint32_t(vertices.size()) - 1);
			}

			indices.push_back(tempIndices[0]);
			if (flipAxisAndWinding)
			{
				indices.push_back(tempIndices[2]);
				indices.push_back(tempIndices[1]);
			}
			else
			{
				indices.push_back(tempIndices[1]);
				indices.push_back(tempIndices[2]);
			}
		}
		//read till end of line and ignore all remaining chars
		file.ignore(1000, '\n');
	}

	//Cheap Tangent Calculations
	/*for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t index0 = indices[i];
		uint32_t index1 = indices[size_t(i) + 1];
		uint32_t index2 = indices[size_t(i) + 2];

		const glm::vec2& p0 = vertices[index0].position;
		const glm::vec2& p1 = vertices[index1].position; 
		const glm::vec2& p2 = vertices[index2].position; 
		const glm::vec2& uv0 = vertices[index0].uv; 
		const glm::vec2& uv1 = vertices[index1].uv; 
		const glm::vec2& uv2 = vertices[index2].uv; 

		const glm::vec2 edge0 = p1 - p0;
		const glm::vec2 edge1 = p2 - p0;
		const glm::vec2 diffX = glm::vec2(uv1.x - uv0.x, uv2.x - uv0.x); 
		const glm::vec2 diffY = glm::vec2(uv1.y - uv0.y, uv2.y - uv0.y);
		float r = 1.f / Vector2::Cross(diffX, diffY);

		glm::vec2 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
		vertices[index0].tangent += tangent;
		vertices[index1].tangent += tangent;
		vertices[index2].tangent += tangent;
	}*/

	//Fix the tangents per vertex now because we accumulated
	/*for (auto& v : vertices)
	{
		v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

		if (flipAxisAndWinding)
		{
			v.position.z *= -1.f;
			v.normal.z *= -1.f;
			v.tangent.z *= -1.f;
		}

	}*/

	return true;
}

uint32_t GP2_Mesh::FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
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
