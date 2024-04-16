#include "GP2_Mesh.h"
#include "vulkanbase/VulkanUtil.h"
#include "GP2_UniformBufferObject.h"

GP2_Mesh::GP2_Mesh(VkDevice device, VkPhysicalDevice physicalDevice) :
	m_Device{ device },
	m_PhysicalDevice{ physicalDevice },
	m_VkVertexBuffer{},
	m_VkVertexBufferMemory{},
	m_VkIndexBuffer{},
	m_VkIndexBufferMemory{},
	m_VertexConstant{}
{
}

void GP2_Mesh::Initialize(VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	CreateVertexBuffer(m_MeshVertices, graphicsQueue, queueFamilyIndices); 
	CreateIndexBuffer(m_MeshIndices, graphicsQueue, queueFamilyIndices); 

	m_pVertexBuffer = std::make_unique<GP2_Buffer>(m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(Vertex) * m_MeshVertices.size()); 
	m_pVertexBuffer->Upload(sizeof(Vertex) * m_MeshVertices.size(), m_MeshVertices.data());
	
	m_pIndexBuffer = std::make_unique<GP2_Buffer>(m_Device, m_PhysicalDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(uint16_t) * m_MeshIndices.size());
	m_pIndexBuffer->Upload(sizeof(uint16_t) * m_MeshIndices.size(), m_MeshIndices.data());  
}

void GP2_Mesh::DestroyMesh()
{
	DestroyIndexBuffer(); 
	DestroyVertexBuffer(); 

	m_pVertexBuffer->Destroy();
	m_pIndexBuffer->Destroy();
}

void GP2_Mesh::DestroyVertexBuffer()
{
	vkDestroyBuffer(m_Device, m_VkVertexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VkVertexBufferMemory, nullptr);
}

void GP2_Mesh::DestroyIndexBuffer()
{
	vkDestroyBuffer(m_Device, m_VkIndexBuffer, nullptr);
	vkFreeMemory(m_Device, m_VkIndexBufferMemory, nullptr);
}

void GP2_Mesh::Draw(VkPipelineLayout pipelineLayout, VkCommandBuffer buffer)
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

void GP2_Mesh::AddVertex(const glm::vec2 pos, const glm::vec3 color)
{
	m_MeshVertices.push_back(Vertex{ pos, color }); 
}

void GP2_Mesh::AddIndices(const std::vector<uint16_t> indices)
{
	m_MeshIndices = indices;
}

void GP2_Mesh::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(m_Device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(m_PhysicalDevice, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(m_Device, buffer, bufferMemory, 0);
}

void GP2_Mesh::CreateVertexBuffer(std::vector<Vertex> vertices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data{};
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), size_t(bufferSize));
	vkUnmapMemory(m_Device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		m_VkVertexBuffer, m_VkVertexBufferMemory);

	CopyBuffer(stagingBuffer, m_VkVertexBuffer, bufferSize, graphicsQueue, queueFamilyIndices);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void GP2_Mesh::CreateIndexBuffer(const std::vector<uint16_t> indices, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data{};
	vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_Device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VkIndexBuffer, m_VkIndexBufferMemory);

	CopyBuffer(stagingBuffer, m_VkIndexBuffer, bufferSize, graphicsQueue, queueFamilyIndices);

	vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
	vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
}

void GP2_Mesh::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue graphicsQueue, QueueFamilyIndices queueFamilyIndices)
{
	GP2_CommandPool commandPool{};
	commandPool.Initialize(m_Device, queueFamilyIndices);

	GP2_CommandBuffer commandBuffer{ commandPool.CreateCommandBuffer() };

	commandBuffer.BeginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer.GetVkCommandBuffer(), srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer.EndRecording();

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	commandBuffer.Sumbit(submitInfo);
	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	const VkCommandBuffer commandBufferVk = commandBuffer.GetVkCommandBuffer();
	vkFreeCommandBuffers(m_Device, commandPool.GetVkCommandPool(), 1, &commandBufferVk);

	commandPool.Destroy();
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
