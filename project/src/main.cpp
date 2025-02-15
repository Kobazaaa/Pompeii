#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

constexpr uint32_t g_WIDTH = 800;
constexpr uint32_t g_HEIGHT = 600;

const std::vector<const char*> g_VALIDATION_LAYERS = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	constexpr bool g_ENABLE_VALIDATION_LAYERS = false;
#else
	constexpr bool g_ENABLE_VALIDATION_LAYERS = true;
#endif

class HelloTriangleApplication
{
public:
	void Run()
	{
		InitWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

private:
	void InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_pWindow = glfwCreateWindow(g_WIDTH, g_HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void InitVulkan()
	{
		CreateInstance();
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(m_pWindow))
		{
			glfwPollEvents();
		}
	}

	void Cleanup()
	{
		vkDestroyInstance(m_Instance, nullptr);

		glfwDestroyWindow(m_pWindow);

		glfwTerminate();
	}

	void CreateInstance()
	{
		if (g_ENABLE_VALIDATION_LAYERS && !CheckValidationLayerSupport())
			throw std::runtime_error("Validation Layers requested, but not available!");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;


		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		if (g_ENABLE_VALIDATION_LAYERS)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(g_VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = g_VALIDATION_LAYERS.data();
		}
		else createInfo.enabledLayerCount = 0;
		
		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create instance!");

		// List all available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "Available extensions:\n";
		for (const auto& extension : extensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}

	}

	bool CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : g_VALIDATION_LAYERS)
		{
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	GLFWwindow* m_pWindow{ nullptr };

	VkInstance m_Instance{ VK_NULL_HANDLE };
};

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}