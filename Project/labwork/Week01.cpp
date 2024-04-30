#define GLM_FORCE_RADIANS 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
		m_CameraPosition += m_CameraForward * 10.f; 
	}
	if (key == GLFW_KEY_S && (action == GLFW_REPEAT || action == GLFW_PRESS))  
	{ 
		m_CameraPosition -= m_CameraForward * 10.f;   
	} 
	if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))  
	{  
		m_CameraPosition -= m_CameraRight * 10.f;  
	} 
	if (key == GLFW_KEY_D && (action == GLFW_REPEAT || action == GLFW_PRESS))  
	{ 
		m_CameraPosition += m_CameraRight * 10.f; 
	}
}
void VulkanBase::MouseMove(GLFWwindow * window, double xpos, double ypos)
{
	glm::vec2 currentMousePosition = glm::vec2(xpos, ypos);
	glm::vec2 mouseDelta = currentMousePosition - m_LastMousePosition;

	m_LastMousePosition = currentMousePosition;
	
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	if (state == GLFW_PRESS)
	{
		const float sensitivity{ 0.001f };

		m_Yaw += mouseDelta.x * sensitivity;  
		m_Pitch += mouseDelta.y * sensitivity; 

		if (m_Pitch > glm::radians(89.0f))
		{
			m_Pitch = glm::radians(89.0f);
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

glm::mat4 VulkanBase::UpdateCamera()
{
	glm::vec3 newForward{ glm::normalize(glm::rotate(glm::mat4(1.0f), -m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f)) *
					      glm::rotate(glm::mat4(1.0f), m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f)) *
					      glm::vec4(m_CameraForward, 1.0f))
	};

	m_CameraForward = newForward; 
	m_CameraRight = glm::cross(m_CameraForward, glm::vec3(0.f, 1.f, 0.f));
	m_CameraUp = glm::cross(m_CameraRight, m_CameraForward);

	glm::mat4 viewMatrix{ glm::lookAt(m_CameraPosition, m_CameraPosition + m_CameraForward, m_CameraUp) };

	return viewMatrix; 
}
