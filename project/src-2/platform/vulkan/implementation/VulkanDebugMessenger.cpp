// -- Pompeii Includes --
#include "VulkanDebugMessenger.h"

// -- Output Includes --
#include <iostream>
#include <stdexcept>
#include "ConsoleTextSettings.h"

// -- Standard Library --
#include <vector>


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::VulkanDebugMessenger::VulkanDebugMessenger(VkInstance instance)
	: m_Instance(instance)
{
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugMessenger::Initialize()
{
	SetupMessenger();
}
void pompeii::vulkan::VulkanDebugMessenger::Shutdown()
{
	if (m_Messenger != VK_NULL_HANDLE && PFN_DestroyDebugUtilsMessengerEXT)
	{
		PFN_DestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
		m_Messenger = VK_NULL_HANDLE;
	}
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugMessenger::AddValidationLayer(const char* layer)
{
	m_vValidationLayers.push_back(layer);
}
bool pompeii::vulkan::VulkanDebugMessenger::CheckValidationLayerSupport() const
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

const std::vector<const char*>& pompeii::vulkan::VulkanDebugMessenger::GetValidationLayers()
{
	return m_vValidationLayers;
}

//--------------------------------------------------
//    Helper
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugMessenger::SetupMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	PFN_CreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
		(vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT"));

	PFN_DestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
		(vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"));

	if (PFN_CreateDebugUtilsMessengerEXT)
	{
		VkResult result = PFN_CreateDebugUtilsMessengerEXT(
			m_Instance, &createInfo, nullptr, &m_Messenger);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create debug messenger!");
	}
	else
	{
		throw std::runtime_error("Failed to find vkCreateDebugUtilsMessengerEXT!");
	}
}
void pompeii::vulkan::VulkanDebugMessenger::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = MessageCallback;
	createInfo.pUserData = nullptr;
}


//--------------------------------------------------
//    Callback
//--------------------------------------------------
VkBool32 pompeii::vulkan::VulkanDebugMessenger::MessageCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*)
{
	std::string type;
	const char* severity;
	const char* txtColor;
	const char* bgColor = "";

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		severity = "ERROR";
		txtColor = ERROR_TXT;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		severity = "WARNING";
		txtColor = WARNING_TXT;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		severity = "INFO";
		txtColor = INFO_TXT;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		severity = "VERBOSE";
		txtColor = VERBOSE_TXT;
	}
	else
	{
		severity = "UNKNOWN";
		txtColor = WHITE_TXT;
	}

	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		type += "GENERAL ";
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		type += "VALIDATION ";
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		type += "PERFORMANCE ";
	if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
		type += "DEVICE ADDRESS BINDING ";

	std::cerr << bgColor << txtColor
		<< "--- Vulkan Debug Messenger - [ " << severity << " | " << type << "]\n"
		<< "-> " << pCallbackData->pMessage << "\n"
		<< "----------------------" << RESET_TXT << "\n\n";

	return VK_FALSE;
}
