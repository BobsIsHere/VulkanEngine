#include "vulkanbase/VulkanBase.h"
#include "GP2_Mesh.h"

void VulkanBase::initWindow() 
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VulkanBase::drawScene() 
{
	m_TriangleMesh->Draw(m_CommandBuffer.GetVkCommandBuffer());
	m_RectangleMesh->Draw(m_CommandBuffer.GetVkCommandBuffer());
	m_OvalMesh->Draw(m_CommandBuffer.GetVkCommandBuffer());
}