#include "vulkanbase/VulkanBase.h"
#include "GP2_Mesh.h"

void VulkanBase::InitWindow() 
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

//void VulkanBase::DrawScene() 
//{
//	m_TriangleMesh->Draw(m_PipelineLayout, m_CommandBuffer.GetVkCommandBuffer());
//	m_RectangleMesh->Draw(m_PipelineLayout, m_CommandBuffer.GetVkCommandBuffer());
//	m_OvalMesh->Draw(m_PipelineLayout, m_CommandBuffer.GetVkCommandBuffer());
//}