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
#include "GP2_DescriptorPool.h"
#include "GP2_GraphicsPipeline.h"

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
		
		m_GP2D = std::make_unique<GP2_GraphicsPipeline>("shaders/shader.vert.spv", "shaders/shader.frag.spv");
		//m_GP3D = std::make_unique<GP2_GraphicsPipeline>("shaders/shader.vert.spv", "shaders/shader.frag.spv");

		// week 03
		m_GradientShader.Initialize(m_Device);

		m_DescriptorPool = std::make_unique<GP2_DescriptorPool<GP2_UniformBufferObject<VertexUBO>>>(m_Device, MAX_FRAMES_IN_FLIGHT);
		m_DescriptorPool->Initialize({m_Device, m_PhysicalDevice});

		//Draw Triangle
		m_TriangleMesh = std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice);

		m_TriangleMesh->AddVertex({ -0.25f, 0.75f }, { 0.f, 1.f, 0.f }); // 0
		m_TriangleMesh->AddVertex({ 0.25f, 0.75f }, { 0.f, 0.f, 1.f }); // 1
		m_TriangleMesh->AddVertex({ 0, 0.25f }, { 0.f, 1.f, 0.f }); // 2

		std::vector<uint16_t> triangleIndices{ 2, 1, 0 };
		m_TriangleMesh->AddIndices(triangleIndices);

		m_TriangleMesh->Initialize(m_GraphicsQueue, findQueueFamilies(m_PhysicalDevice));

		//Draw Rectangle
		m_RectangleMesh = std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice);

		m_RectangleMesh->AddVertex({ 0.25f, -0.25f }, { 0.25f, 0.75f, 0.25f }); // 0
		m_RectangleMesh->AddVertex({ 0.75f, -0.25f }, { 0.25f, 0.75f, 0.25f }); // 1
		m_RectangleMesh->AddVertex({ 0.25f, -0.75f }, { 0.75f, 0.25f, 0.75f }); // 2
		m_RectangleMesh->AddVertex({ 0.75f, -0.75f }, { 0.75f, 0.25f, 0.75f }); // 3

		std::vector<uint16_t> rectIndices{ 2, 1, 0, 3, 1, 2 };
		m_RectangleMesh->AddIndices(rectIndices);

		m_RectangleMesh->Initialize(m_GraphicsQueue, findQueueFamilies(m_PhysicalDevice));

		//Draw Oval
		m_OvalMesh = std::make_unique<GP2_Mesh>(m_Device, m_PhysicalDevice);

		m_OvalMesh->AddVertex({ -0.625f, -0.625f }, { 1.f, 1.f, 0.f }); // 0
		m_OvalMesh->AddVertex({ -0.625f, -0.375f }, { 0.f, 1.f, 0.f }); // 1
		m_OvalMesh->AddVertex({ -0.5f, -0.25f }, { 0.f, 1.f, 1.f }); // 2
		m_OvalMesh->AddVertex({ -0.375f, -0.375f }, { 0.f, 0.f, 1.f }); // 3
		m_OvalMesh->AddVertex({ -0.375f, -0.625f }, { 1.f, 0.f, 1.f }); // 4
		m_OvalMesh->AddVertex({ -0.5f, -0.75f }, { 1.f,0.f,0.f }); // 5
		m_OvalMesh->AddVertex({ -0.5f, -0.5f }, { 1.f,1.f,1.f }); // center, 6

		std::vector<uint16_t> ovalIndices{ 0, 1, 6, 1, 2, 6, 2, 3, 6, 3, 4, 6, 4, 5, 6, 5, 0, 6 };
		m_OvalMesh->AddIndices(ovalIndices); 

		m_OvalMesh->Initialize(m_GraphicsQueue, findQueueFamilies(m_PhysicalDevice));
		
		CreateUniformBuffer();
		createRenderPass(); 
		createGraphicsPipeline(); 
		createFrameBuffers(); 

		// week 02
		m_CommandPool.Initialize(m_Device, findQueueFamilies(m_PhysicalDevice)); 
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
		vkDeviceWaitIdle(m_Device);
	}

	void cleanup() 
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(m_Device, m_RenderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(m_Device, m_ImageAvailableSemaphore[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFence[i], nullptr);
		}

		m_CommandPool.Destroy();

		for (auto framebuffer : m_SwapChainFramebuffers) 
		{
			vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
		}

		vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
		vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

		for (auto imageView : m_SwapChainImageViews) 
		{
			vkDestroyImageView(m_Device, imageView, nullptr);
		}

		m_TriangleMesh->DestroyMesh();
		m_RectangleMesh->DestroyMesh();
		m_OvalMesh->DestroyMesh(); 

		for (size_t i = 0; i < m_UniformBuffers.size(); i++) 
		{
			m_UniformBuffers[i]->Destroy();
		}

		if (enableValidationLayers) 
		{
			DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}

		vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);
		vkDestroyDevice(m_Device, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
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

	GP2_Shader m_GradientShader
	{
		"shaders/shader.vert.spv",
		"shaders/shader.frag.spv"
	};

	std::unique_ptr<GP2_GraphicsPipeline> m_GP2D; 
	//std::unique_ptr<GP2_GraphicsPipeline> m_GP3D;  
	std::unique_ptr< GP2_DescriptorPool< GP2_UniformBufferObject<VertexUBO> > > m_DescriptorPool;   

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
	
	std::unique_ptr<GP2_Mesh> m_TriangleMesh;
	std::unique_ptr<GP2_Mesh> m_RectangleMesh;
	std::unique_ptr<GP2_Mesh> m_OvalMesh;
	
	std::vector<VkFramebuffer> m_SwapChainFramebuffers;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkRenderPass m_RenderPass;
	 
	std::vector<GP2_Buffer*> m_UniformBuffers;  
	std::vector<void*> m_UniformBuffersMapped; 

	void createFrameBuffers();
	void createRenderPass();
	void createGraphicsPipeline();
	void CreateUniformBuffer();

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

	VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentQueue;
	
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	void createLogicalDevice();

	// Week 06
	// Main initialization

	VkInstance m_Instance;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkDevice m_Device = VK_NULL_HANDLE;
	VkSurfaceKHR m_Surface;

	std::vector<VkSemaphore> m_ImageAvailableSemaphore;
	std::vector<VkSemaphore> m_RenderFinishedSemaphore; 
	std::vector<VkFence> m_InFlightFence;

	uint32_t m_CurrentFrame{ 0 };

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void createInstance();

	void UpdateUniformBuffer(uint32_t currentImage);
	void createSyncObjects();
	void drawFrame();
	void BeginRenderPass(const GP2_CommandBuffer& buffer, VkFramebuffer currentBuffer, VkExtent2D extent);
	void EndRenderPass(const GP2_CommandBuffer& buffer);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
};