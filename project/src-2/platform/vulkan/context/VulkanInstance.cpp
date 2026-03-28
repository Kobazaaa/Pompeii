// -- Standard Library --
#include <iostream>
#include <stdexcept>

// -- Pompeii Includes --
#include "VulkanInstance.h"
#include "VulkanContext.h"
#include "RenderDebugger.h"
#include "ConsoleTextSettings.h"

using namespace pompeii::vulkan;

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  VulkanInstance
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
Instance::~Instance()
{
	if (m_Instance)
		vkDestroyInstance(m_Instance, nullptr);
}

//--------------------------------------------------
//    Operation Overloads
//--------------------------------------------------
Instance::operator VkInstance() const{ return m_Instance; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  INSTANCE BUILDER
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
InstanceBuilder::InstanceBuilder()
{
	m_AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	m_AppInfo.pApplicationName = "";
	m_AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	m_AppInfo.pEngineName = "";
	m_AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	m_AppInfo.apiVersion = VK_API_VERSION_1_0;
}

//--------------------------------------------------
//    Builder
//--------------------------------------------------
InstanceBuilder& InstanceBuilder::SetApplicationName(const std::string& name)	{ m_AppInfo.pApplicationName = name.c_str(); return *this; }
InstanceBuilder& InstanceBuilder::SetEngineName(const std::string& name)		{ m_AppInfo.pEngineName = name.c_str(); return *this; }
InstanceBuilder& InstanceBuilder::SetAPIVersion(uint32_t apiVersion)			{ m_AppInfo.apiVersion = apiVersion; return *this; }
InstanceBuilder& InstanceBuilder::AddInstanceExtension(const char* extName)		{ m_vInstanceExtensions.push_back(extName); return *this; }

void InstanceBuilder::Build(VulkanContext& context)
{
	if (RenderDebugger::IsEnabled() && !RenderDebugger::CheckValidationLayerSupport())
		throw std::runtime_error("Validation Layers requested, but not available!");
	if (RenderDebugger::IsEnabled())
		m_vInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	// Setup CreateInfo
	m_CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	m_CreateInfo.pApplicationInfo = &m_AppInfo;

	m_CreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_vInstanceExtensions.size());
	m_CreateInfo.ppEnabledExtensionNames = m_vInstanceExtensions.data();
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (RenderDebugger::IsEnabled())
	{
		m_CreateInfo.enabledLayerCount = RenderDebugger::GetNumberOfLayers();
		m_CreateInfo.ppEnabledLayerNames = RenderDebugger::GetValidationLayers().data();

		RenderDebugger::PopulateDebugMessengerCreateInfo(debugCreateInfo);
		m_CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		m_CreateInfo.enabledLayerCount = 0;
		m_CreateInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&m_CreateInfo, nullptr, &context.instance.m_Instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create instance!");

	if (RenderDebugger::IsEnabled())
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

