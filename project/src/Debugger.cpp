// -- Standard Library
#include <iostream>
#include <ostream>
#include <stdexcept>

// -- Pompeii Includes --
#include "Debugger.h"
#include "Context.h"
#include "ConsoleTextSettings.h"


//--------------------------------------------------
//    Setup
//--------------------------------------------------
void pom::Debugger::SetEnabled(bool enabled)				{ m_IsEnabled = enabled; }
void pom::Debugger::AddValidationLayer(const char* layer)	{ m_vValidationLayers.push_back(layer); }

void pom::Debugger::Setup(const Context& context)
{
	if (!m_IsEnabled) return;

	SetupInstanceDebugUtils(context);
	SetupDeviceDebugUtils(context);
}


void pom::Debugger::Destroy()
{
	if (!m_IsEnabled) return;

	if (m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT)
		m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT(m_InstanceDebugUtils.instance, m_InstanceDebugUtils.debugUtilsMessenger, nullptr);
}

void pom::Debugger::SetDebugObjectName(uint64_t objectHandle, VkObjectType objectType, const std::string& name)
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = objectType;
	nameInfo.objectHandle = objectHandle;
	nameInfo.pObjectName = name.c_str();

	m_DeviceDebugUtils.setDebugUtilsObjectNameEXT(m_DeviceDebugUtils.device, &nameInfo);
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
						   | VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT
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
VkBool32 pom::Debugger::DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void*)
{
	const char* type = "";
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
		type = "GENERAL";
	else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		type = "VALIDATION";
	else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		type = "PERFORMANCE";
	else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
		type = "DEVICE ADDRESS BINDING";

	std::cerr << bgColor << txtColor
		<< "--- Vulkan Debug - [" << severity << " | " << type << "]\n"
		<< "-> " << pCallbackData->pMessage << "\n"
		<< "----------------------" << RESET_TXT << "\n\n";

	return VK_FALSE;
}

void pom::Debugger::SetupInstanceDebugUtils(const Context& context)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	PopulateDebugMessengerCreateInfo(createInfo);

	m_InstanceDebugUtils.instance = context.instance.GetHandle();
	m_InstanceDebugUtils.createDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>
															(vkGetInstanceProcAddr(context.instance.GetHandle(), "vkCreateDebugUtilsMessengerEXT"));
	m_InstanceDebugUtils.destroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>
															(vkGetInstanceProcAddr(context.instance.GetHandle(), "vkDestroyDebugUtilsMessengerEXT"));

	if (m_InstanceDebugUtils.createDebugUtilsMessengerEXT != nullptr)
		m_InstanceDebugUtils.createDebugUtilsMessengerEXT(context.instance.GetHandle(), &createInfo, nullptr, &m_InstanceDebugUtils.debugUtilsMessenger);
	else
		throw std::runtime_error("Failed to find vkCreateDebugUtilsMessengerEXT!");
}

void pom::Debugger::SetupDeviceDebugUtils(const Context& context)
{
	m_DeviceDebugUtils.device = context.device.GetHandle();

	m_DeviceDebugUtils.setDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>
														(vkGetDeviceProcAddr(context.device.GetHandle(), "vkSetDebugUtilsObjectNameEXT"));
	m_DeviceDebugUtils.cmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>
														(vkGetDeviceProcAddr(context.device.GetHandle(), "vkCmdBeginDebugUtilsLabelEXT"));
	m_DeviceDebugUtils.cmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>
														(vkGetDeviceProcAddr(context.device.GetHandle(), "vkCmdEndDebugUtilsLabelEXT"));
	m_DeviceDebugUtils.cmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>
														(vkGetDeviceProcAddr(context.device.GetHandle(), "InsertDebugUtilsLabelEXT"));
}

