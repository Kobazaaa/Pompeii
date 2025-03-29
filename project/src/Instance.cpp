#include "Instance.h"

#include <iostream>
#include <stdexcept>

#include "GLFW/glfw3.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  INSTANCE	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Instance::Instance(VkInstance instance)
	: m_Instance(instance)
{
#ifdef NDEBUG
	Debugger::SetEnabled(false);
#else
	Debugger::SetEnabled(true);
	Debugger::AddValidationLayer("VK_LAYER_KHRONOS_validation");
#endif
}
pom::Instance::~Instance()
{
	if (Debugger::IsEnabled())
		Debugger::DestroyMessenger(*this);
	vkDestroyInstance(m_Instance, nullptr);

}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
VkInstance& pom::Instance::GetInstance() { return m_Instance; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  INSTANCE BUILDER
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::InstanceBuilder& pom::InstanceBuilder::SetApplicationName(const std::string& name) { m_AppInfo.pApplicationName = name.c_str(); return *this; }
pom::InstanceBuilder& pom::InstanceBuilder::SetEngineName(const std::string& name)		{ m_AppInfo.pEngineName = name.c_str(); return *this; }
pom::Instance& pom::InstanceBuilder::Build(Instance& instance)
{
	if (Debugger::IsEnabled() && !Debugger::CheckValidationLayerSupport())
		throw std::runtime_error("Validation Layers requested, but not available!");

	// Set up AppInfo
	m_AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	// pApplicationName set in SetApplicationName
	m_AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	// pEngineName set in SetEngineName
	m_AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	m_AppInfo.apiVersion = VK_API_VERSION_1_0;

	// Setup CreateInfo
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	m_CreateInfo.pApplicationInfo = &m_AppInfo;

	auto extensions = GetRequiredExtensions();
	m_CreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	m_CreateInfo.ppEnabledExtensionNames = extensions.data();
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (Debugger::IsEnabled())
	{
		m_CreateInfo.enabledLayerCount = Debugger::GetNumberOfLayers();
		m_CreateInfo.ppEnabledLayerNames = Debugger::GetValidationLayers().data();

		Debugger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
		m_CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		m_CreateInfo.enabledLayerCount = 0;
		m_CreateInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&m_CreateInfo, nullptr, &instance.GetInstance()) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");

	if (!Debugger::IsEnabled())
	{
		// List all available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		std::cout << "\nAvailable extensions:\n";
		for (const auto& extension : availableExtensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}
	}

	return instance;
}

std::vector<const char*> pom::InstanceBuilder::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (Debugger::IsEnabled())
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}
