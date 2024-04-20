#include "vulkanbase/VulkanBase.h"
#include "GP2_Mesh.h"

void VulkanBase::InitWindow() 
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->KeyEvent(key, scancode, action, mods);
	});

	glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xpos, double ypos) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->MouseMove(window, xpos, ypos);
	});

	glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
		void* pUser = glfwGetWindowUserPointer(window);
		VulkanBase* vBase = static_cast<VulkanBase*>(pUser);
		vBase->MouseEvent(window, button, action, mods);
	});
}

void VulkanBase::KeyEvent(int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition += std::max(3.0f, m_Radius - 0.2f);
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition -= std::min(30.0f, m_Radius + 0.2f);
	}
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition += std::max(3.0f, m_Radius - 0.2f);
	}
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))
	{
		m_CameraPosition -= std::min(30.0f, m_Radius + 0.2f);
	}
}
void VulkanBase::MouseMove(GLFWwindow * window, double xpos, double ypos)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		float dx = static_cast<float>(xpos) - m_LastMousePosition.x;
		if (dx > 0) 
		{
			m_Rotation += 0.01;
		}
		else 
		{
			m_Rotation -= 0.01;
		}
	}
}
void VulkanBase::MouseEvent(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		std::cout << "right mouse button pressed\n";
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		m_LastMousePosition.x = static_cast<float>(xpos);
		m_LastMousePosition.y = static_cast<float>(ypos);
	}
}