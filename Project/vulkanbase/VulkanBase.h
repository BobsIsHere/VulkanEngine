#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "VulkanUtil.h"

#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <memory>

#include "GP2_Mesh.h"
#include "GP2_CommandPool.h"
#include "GP2_CommandBuffer.h"
#include "GP2_DescriptorPool.h"
#include "GP2_2DGraphicsPipeline.h"
#include "GP2_3DGraphicsPipeline.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails 
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanBase 
{
public:
	void run() 
	{
		InitWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan() 
	{
		// week 06
		CreateInstance();
		SetupDebugMessenger();
		createSurface();

		// week 05
		PickPhysicalDevice();
		CreateLogicalDevice();

		// week 04 
		CreateSwapChain();
		CreateImageViews();

		// week 02
		m_CommandPool.Initialize(m_Device, FindQueueFamilies(m_PhysicalDevice)); 
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer(); 

		// GRAPHICS PIPELINE 2D
		std::unique_ptr<GP2_Mesh> m_RectangleMesh{ std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice) };
		std::unique_ptr<GP2_Mesh> m_OvalMesh{ std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice) };
		
		//Draw Rectangle
		m_RectangleMesh->AddVertex({ 0.25f, -0.25f, 0.f }, { 0.25f, 0.75f, 0.25f }); // 0
		m_RectangleMesh->AddVertex({ 0.75f, -0.25f, 0.f }, { 0.25f, 0.75f, 0.25f }); // 1
		m_RectangleMesh->AddVertex({ 0.25f, -0.75f, 0.f }, { 0.75f, 0.25f, 0.75f }); // 2
		m_RectangleMesh->AddVertex({ 0.75f, -0.75f, 0.f }, { 0.75f, 0.25f, 0.75f }); // 3

		const std::vector<uint16_t> rectIndices{ 2, 1, 0, 3, 1, 2 };
		m_RectangleMesh->AddIndices(rectIndices);

		m_RectangleMesh->Initialize(m_GraphicsQueue, FindQueueFamilies(m_PhysicalDevice));
		m_GP2D.AddMesh(std::move(m_RectangleMesh));

		//Draw Oval
		m_OvalMesh->AddVertex({ 0.71f / 4 - 0.5f, 0.71f / 4 - 0.5f, 0.f }, { 1.f, 1.f, 0.f }); // 0
		m_OvalMesh->AddVertex({ 0.f - 0.5f, 1.f / 4 - 0.5f, 0.f }, { 0.f, 1.f, 0.f }); // 1
		m_OvalMesh->AddVertex({ -0.71f / 4 - 0.5f, 0.71f / 4 - 0.5f, 0.f }, { 0.f, 1.f, 1.f }); // 2
		m_OvalMesh->AddVertex({ -1.f / 4 - 0.5f, 0.f - 0.5f, 0.f }, { 0.f, 0.f, 1.f }); // 3
		m_OvalMesh->AddVertex({ -0.71f / 4 - 0.5f, -0.71f / 4 - 0.5f, 0.f }, { 1.f, 0.f, 1.f }); // 4
		m_OvalMesh->AddVertex({ 0.f / 4 - 0.5f, -1.f / 4 - 0.5f, 0.f }, { 1.f,0.f,0.f }); // 5
		m_OvalMesh->AddVertex({ 0.71f / 4 - 0.5f, -0.71f / 4 - 0.5f, 0.f }, { 1.f, 1.f, 0.f }); // 6
		m_OvalMesh->AddVertex({ 1.f / 4 - 0.5f, 0.f / 4 - 0.5f, 0.f }, { 0.f, 1.f, 0.f }); // 7
		m_OvalMesh->AddVertex({ 0.f - 0.5f, 0.f - 0.5f, 0.f }, { 1.f, 1.f, 1.f }); // center, 8

		const std::vector<uint16_t> ovalIndices{ 
			0, 8, 1,
			1, 8, 2,
			2, 8, 3,
			3, 8, 4,
			4, 8, 5,
			5, 8, 6,
			6, 8, 7,
			7, 8, 0 
		};
		m_OvalMesh->AddIndices(ovalIndices);

		m_OvalMesh->Initialize(m_GraphicsQueue, FindQueueFamilies(m_PhysicalDevice));
		m_GP2D.AddMesh(std::move(m_OvalMesh));

		// GRAPHICS PIPELINE 3D
		std::unique_ptr<GP2_Mesh> m_BunnyMesh{ std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice) };
		std::unique_ptr<GP2_Mesh> m_BirdHouseMesh{ std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice) };

		//Draw Bunny
		std::vector<glm::vec3> bunnyVertices{};
		std::vector<glm::vec3> bunnyNormals{};
		std::vector<uint16_t> bunnyIndices{};

		m_BunnyMesh->ParseOBJ("resources/bunny.obj", bunnyVertices, bunnyNormals, bunnyIndices);
		m_BunnyMesh->AddVertices(bunnyVertices, bunnyNormals, {0.5f, 0.5f, 0.5f}); 
		m_BunnyMesh->AddIndices(bunnyIndices);  

		m_BunnyMesh->Initialize(m_GraphicsQueue, FindQueueFamilies(m_PhysicalDevice));
		m_GP3D.AddMesh(std::move(m_BunnyMesh)); 

		//Draw BirdHouse
		std::vector<glm::vec3> birdHouseVertices{};
		std::vector<glm::vec3> birdHouseNormals{};
		std::vector<uint16_t> birdHouseIndices{};

		m_BirdHouseMesh->ParseOBJ("resources/birdhouse.obj", birdHouseVertices, birdHouseNormals, birdHouseIndices);
		m_BirdHouseMesh->AddVertices(birdHouseVertices, birdHouseNormals, { 0.5f, 0.5f, 0.5f });
		m_BirdHouseMesh->AddIndices(birdHouseIndices); 

		m_BirdHouseMesh->Initialize(m_GraphicsQueue, FindQueueFamilies(m_PhysicalDevice));
		m_GP3D.AddMesh(std::move(m_BirdHouseMesh));
		
		CreateRenderPass(); 
		m_GP2D.Initialize(VulkanContext{ m_Device, m_PhysicalDevice, m_RenderPass, m_SwapChainExtent }); 
		m_GP3D.Initialize(VulkanContext{ m_Device, m_PhysicalDevice, m_RenderPass, m_SwapChainExtent }); 
		CreateFrameBuffers(); 

		// week 06
		CreateSyncObjects();
	}

	void mainLoop() 
	{
		while (!glfwWindowShouldClose(m_Window)) 
		{
			glfwPollEvents();
			// week 06
			DrawFrame();
		}
		vkDeviceWaitIdle(m_Device);
	}

	void cleanup() 
	{
		vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore, nullptr);
		vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore, nullptr);
		vkDestroyFence(m_Device, m_InFlightFence, nullptr);
		
		m_CommandPool.Destroy();  

		for (auto framebuffer : m_SwapChainFramebuffers) 
		{
			vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
		}

		m_GP2D.Cleanup(); 
		m_GP3D.Cleanup();

		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		for (auto imageView : m_SwapChainImageViews) 
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
		vkDestroyDevice(m_Device, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

		if (enableValidationLayers) 
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}

		vkDestroyInstance(m_Instance, nullptr);

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void createSurface() 
	{
		if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create window surface!");
		}
	}

	// Graphics Pipelines
	GP2_2DGraphicsPipeline<ViewProjection> m_GP2D{ "shaders/shader.vert.spv", "shaders/shader.frag.spv" };   
	GP2_2DGraphicsPipeline<VertexUBO> m_GP3D{ "shaders/objshader.vert.spv", "shaders/objshader.frag.spv" };   

	// Camera
	glm::vec2 m_LastMousePosition{ 0.f, 0.f };

	glm::vec3 m_Target{ 0.f, 0.f, 0.f };
	glm::vec3 m_CameraPosition{ 0.f, 0.f, 3.f };
	glm::vec3 m_CameraForward{ 0.f, 0.f, -1.f };
	glm::vec3 m_CameraUp{ 0.f, 1.f, 0.f };
	glm::vec3 m_CameraRight{ 1.f, 0.f, 0.f };

	const float m_FOV{ 45.f };
	const float m_AspectRatio{ m_SwapChainExtent.width / static_cast<float>(m_SwapChainExtent.height) };
	const float m_Radius{ 5.f };
	float m_Rotation{ 0.f };

	// Week 01: 
	// Actual window
	// simple fragment + vertex shader creation functions
	// These 5 functions should be refactored into a separate C++ class
	// with the correct internal state.

	GLFWwindow* m_Window;
	void InitWindow();

	void KeyEvent(int key, int scancode, int action, int mods);
	void MouseMove(GLFWwindow* window, double xpos, double ypos);
	void MouseEvent(GLFWwindow* window, int button, int action, int mods);

	// Week 02
	// Queue families
	// CommandBuffer concept

	GP2_CommandPool m_CommandPool;
	GP2_CommandBuffer m_CommandBuffer;

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	
	// Week 03
	// Renderpass concept
		
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkRenderPass m_RenderPass;

	void CreateFrameBuffers(); 
	void CreateRenderPass(); 

	// Week 04
	// Swap chain and image view support

	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

	std::vector<VkImageView> m_SwapChainImageViews;

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	void CreateSwapChain();
	void CreateImageViews();

	// Week 05 
	// Logical and physical device

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
	
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	void CreateLogicalDevice();

	// Week 06
	// Main initialization

	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface;

	VkSemaphore m_ImageAvailableSemaphore; 
	VkSemaphore m_RenderFinishedSemaphore; 
	VkFence m_InFlightFence;

	uint32_t m_CurrentFrame{ 0 };

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void SetupDebugMessenger();
	std::vector<const char*> GetRequiredExtensions();
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	void CreateInstance();

	void CreateSyncObjects();
	void DrawFrame();
	void BeginRenderPass(const GP2_CommandBuffer& buffer, VkFramebuffer currentBuffer, VkExtent2D extent);
	void EndRenderPass(const GP2_CommandBuffer& buffer);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};