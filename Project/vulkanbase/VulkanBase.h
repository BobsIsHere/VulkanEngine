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

#include "GP2_Shader.h"
#include "GP2_Mesh.h"
#include "GP2_CommandPool.h"
#include "GP2_CommandBuffer.h"

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices 
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
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
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initVulkan() 
	{
		// week 06
		createInstance();
		setupDebugMessenger();
		createSurface();

		// week 05
		pickPhysicalDevice();
		createLogicalDevice();

		// week 04 
		createSwapChain();
		createImageViews();
		
		// week 03
		m_GradientShader.Initialize(device);

		//Draw Triangle
		m_TriangleMesh = std::make_unique<GP2_Mesh>(device, physicalDevice);

		m_TriangleMesh->AddVertex({ -0.25f, 0.75f }, { 0.f, 1.f, 0.f }); // 0
		m_TriangleMesh->AddVertex({ 0.25f, 0.75f }, { 0.f, 0.f, 1.f }); // 1
		m_TriangleMesh->AddVertex({ 0, 0.25f }, { 0.f, 1.f, 0.f }); // 2

		std::vector<uint16_t> triangleIndices{ 2, 1, 0 };
		m_TriangleMesh->AddIndices(triangleIndices);

		m_TriangleMesh->Initialize(graphicsQueue, findQueueFamilies(physicalDevice));

		//Draw Rectangle
		m_RectangleMesh = std::make_unique<GP2_Mesh>(device, physicalDevice);

		m_RectangleMesh->AddVertex({ 0.25f, -0.25f }, { 0.25f, 0.75f, 0.25f }); // 0
		m_RectangleMesh->AddVertex({ 0.75f, -0.25f }, { 0.25f, 0.75f, 0.25f }); // 1
		m_RectangleMesh->AddVertex({ 0.25f, -0.75f }, { 0.75f, 0.25f, 0.75f }); // 2
		m_RectangleMesh->AddVertex({ 0.75f, -0.75f }, { 0.75f, 0.25f, 0.75f }); // 3

		std::vector<uint16_t> rectIndices{ 2, 1, 0, 3, 1, 2 };
		m_RectangleMesh->AddIndices(rectIndices);

		m_RectangleMesh->Initialize(graphicsQueue, findQueueFamilies(physicalDevice));

		//Draw Oval
		m_OvalMesh = std::make_unique<GP2_Mesh>(device, physicalDevice);

		m_OvalMesh->AddVertex({ -0.625f, -0.625f }, { 1.f, 1.f, 0.f }); // 0
		m_OvalMesh->AddVertex({ -0.625f, -0.375f }, { 0.f, 1.f, 0.f }); // 1
		m_OvalMesh->AddVertex({ -0.5f, -0.25f }, { 0.f, 1.f, 1.f }); // 2
		m_OvalMesh->AddVertex({ -0.375f, -0.375f }, { 0.f, 0.f, 1.f }); // 3
		m_OvalMesh->AddVertex({ -0.375f, -0.625f }, { 1.f, 0.f, 1.f }); // 4
		m_OvalMesh->AddVertex({ -0.5f, -0.75f }, { 1.f,0.f,0.f }); // 5
		m_OvalMesh->AddVertex({ -0.5f, -0.5f }, { 1.f,1.f,1.f }); // center, 6

		std::vector<uint16_t> ovalIndices{ 0, 1, 6, 1, 2, 6, 2, 3, 6, 3, 4, 6, 4, 5, 6, 5, 0, 6 };
		m_OvalMesh->AddIndices(ovalIndices); 

		m_OvalMesh->Initialize(graphicsQueue, findQueueFamilies(physicalDevice));
		
		createRenderPass();
		createGraphicsPipeline();
		createFrameBuffers();

		// week 02
		m_CommandPool.Initialize(device, findQueueFamilies(physicalDevice)); 
		m_CommandBuffer = m_CommandPool.CreateCommandBuffer(); 

		// week 06
		createSyncObjects();
	}

	void mainLoop() 
	{
		while (!glfwWindowShouldClose(m_Window)) 
		{
			glfwPollEvents();
			// week 06
			drawFrame();
		}
		vkDeviceWaitIdle(device);
	}

	void cleanup() 
	{
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);

		m_CommandPool.Destroy();
		for (auto framebuffer : m_SwapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}

		vkDestroyPipeline(device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(device, m_RenderPass, nullptr);
		for (auto imageView : m_SwapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}

		m_TriangleMesh->DestroyMesh();
		m_RectangleMesh->DestroyMesh();
		m_OvalMesh->DestroyMesh(); 

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		vkDestroySwapchainKHR(device, m_SwapChain, nullptr);
		vkDestroyDevice(device, nullptr);

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);

		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void createSurface() 
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	GP2_Shader m_GradientShader
	{ 
		"shaders/shader.vert.spv", 
		"shaders/shader.frag.spv" 
	};

	// Week 01: 
	// Actual window
	// simple fragment + vertex shader creation functions
	// These 5 functions should be refactored into a separate C++ class
	// with the correct internal state.

	GLFWwindow* m_Window;
	void initWindow();
	void drawScene();

	// Week 02
	// Queue families
	// CommandBuffer concept

	GP2_CommandPool m_CommandPool;
	GP2_CommandBuffer m_CommandBuffer;

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void drawFrame(uint32_t imageIndex);
	
	// Week 03
	// Renderpass concept
	// Graphics pipeline
	std::unique_ptr<GP2_Mesh> m_TriangleMesh;
	std::unique_ptr<GP2_Mesh> m_RectangleMesh;
	std::unique_ptr<GP2_Mesh> m_OvalMesh;
	
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkRenderPass m_RenderPass;

	void createFrameBuffers();
	void createRenderPass();
	void createGraphicsPipeline();

	// Week 04
	// Swap chain and image view support

	VkSwapchainKHR m_SwapChain;
	std::vector<VkImage> m_SwapChainImages;
	VkFormat m_SwapChainImageFormat;
	VkExtent2D m_SwapChainExtent;

	std::vector<VkImageView> m_SwapChainImageViews;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	void createSwapChain();
	void createImageViews();

	// Week 05 
	// Logical and physical device

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	// Week 06
	// Main initialization

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();

	void createSyncObjects();
	void drawFrame();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};