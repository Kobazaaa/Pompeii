#include "Debugger.h"
#include "Instance.h"

#include <iostream>
#include <ostream>
#include <stdexcept>


//--------------------------------------------------
//    Setup
//--------------------------------------------------
void pom::Debugger::SetEnabled(bool enabled)				{ m_IsEnabled = enabled; }
void pom::Debugger::AddValidationLayer(const char* layer)	{ m_vValidationLayers.push_back(layer); }
void pom::Debugger::SetupMessenger(Instance& instance)
{
	if (!m_IsEnabled) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance.GetInstance(), &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		throw std::runtime_error("Failed to set up Debug Messenger!");
}
void pom::Debugger::DestroyMessenger(Instance& instance)
{
	DestroyDebugUtilsMessengerEXT(instance.GetInstance(), m_DebugMessenger, nullptr);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
void pom::Debugger::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
							   | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;
}
bool pom::Debugger::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : m_vValidationLayers)
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

bool pom::Debugger::IsEnabled()											{ return m_IsEnabled; }

const std::vector<const char*>& pom::Debugger::GetValidationLayers()	{ return m_vValidationLayers; }
uint32_t pom::Debugger::GetNumberOfLayers()								{ return static_cast<uint32_t>(m_vValidationLayers.size()); }



//--------------------------------------------------
//    Private
//--------------------------------------------------
VkBool32 pom::Debugger::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
VkResult pom::Debugger::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}
void pom::Debugger::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}
