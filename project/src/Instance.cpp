// -- Standard Library --
#include <iostream>
#include <stdexcept>

// -- Pompeii Includes --
#include "Instance.h"
#include "Debugger.h"
#include "Context.h"
#include "ConsoleTextSettings.h"

// -- GLFW Includes --
#include "GLFW/glfw3.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  INSTANCE	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Instance::Destroy() const { vkDestroyInstance(m_Instance, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkInstance& pom::Instance::GetHandle() const { return m_Instance; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  INSTANCE BUILDER
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------	
pom::InstanceBuilder::InstanceBuilder()
{
	m_AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;			// CAN'T CHANGE
	m_AppInfo.pApplicationName = "";								//? CAN CHANGE
	m_AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);		// CAN'T CHANGE
	m_AppInfo.pEngineName = "";										//? CAN CHANGE
	m_AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);				// CAN'T CHANGE
	m_AppInfo.apiVersion = VK_API_VERSION_1_0;						// CAN'T CHANGE
}

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::InstanceBuilder& pom::InstanceBuilder::SetApplicationName(const std::string& name) { m_AppInfo.pApplicationName = name.c_str(); return *this; }
pom::InstanceBuilder& pom::InstanceBuilder::SetEngineName(const std::string& name)		{ m_AppInfo.pEngineName = name.c_str(); return *this; }
pom::InstanceBuilder& pom::InstanceBuilder::AddInstanceExtension(const char* extName)	{ m_vInstanceExtensions.push_back(extName); return *this; }

void pom::InstanceBuilder::Build(Context& context)
{
	if (Debugger::IsEnabled() && !Debugger::CheckValidationLayerSupport())
		throw std::runtime_error("Validation Layers requested, but not available!");

	// Setup CreateInfo
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	m_CreateInfo.pApplicationInfo = &m_AppInfo;

	GetRequiredExtensions();
	m_CreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_vInstanceExtensions.size());
	m_CreateInfo.ppEnabledExtensionNames = m_vInstanceExtensions.data();
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

	if (vkCreateInstance(&m_CreateInfo, nullptr, &context.instance.m_Instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");

	if (Debugger::IsEnabled())
	{
		// List all available extensions
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

		std::cout << INFO_TXT << "\nAvailable extensions:\n";
		for (const auto& extension : availableExtensions)
		{
			std::cout << '\t' << extension.extensionName << '\n';
		}
		std::cout << "\n" << RESET_TXT;
	}
}

void pom::InstanceBuilder::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (uint32_t index{}; index < glfwExtensionCount; ++index)
		m_vInstanceExtensions.push_back(glfwExtensions[index]);

	if (Debugger::IsEnabled())
		m_vInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
}
