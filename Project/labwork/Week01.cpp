#include "vulkanbase/VulkanBase.h"
#include "GP2_Mesh.h"

void VulkanBase::initWindow() 
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanBase::drawScene() 
{
	/*VkBuffer vertexBuffers[] = { m_VertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(m_CommandBuffer.GetVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
	vkCmdDraw(m_CommandBuffer.GetVkCommandBuffer(), static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);*/

	m_Mesh.Draw(m_CommandBuffer.GetVkCommandBuffer());
}