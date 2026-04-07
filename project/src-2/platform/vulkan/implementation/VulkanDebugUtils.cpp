// -- Pompeii Includes --
#include "VulkanDebugUtils.h"

#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"

// -- Output Includes --
#include <iostream>
#include <stdexcept>
#include "ConsoleTextSettings.h"

// -- Standard Library --
#include <vector>


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::VulkanDebugUtils::VulkanDebugUtils(VkInstance instance, VkDevice device)
	: m_Instance(instance)
	, m_Device(device)
{
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugUtils::Initialize()
{
	SetupMessenger();
	SetupLabelling();
	SetupNames();
}
void pompeii::vulkan::VulkanDebugUtils::Shutdown()
{
	if (m_Messenger != VK_NULL_HANDLE && PFN_DestroyDebugUtilsMessengerEXT)
	{
		PFN_DestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
		m_Messenger = VK_NULL_HANDLE;
	}
}

void pompeii::vulkan::VulkanDebugUtils::BeginLabel(const ICommandBuffer& cmd, const std::string& name,	const glm::vec4& color)
{
	throw std::logic_error("Not implemented");
}
void pompeii::vulkan::VulkanDebugUtils::EndLabel(const ICommandBuffer& cmd)
{
	throw std::logic_error("Not implemented");
}
void pompeii::vulkan::VulkanDebugUtils::InsertLabel(const ICommandBuffer& cmd, const std::string& name,	const glm::vec4& color)
{
	throw std::logic_error("Not implemented");
}



void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const IBuffer& buffer) const
{
	VkBuffer obj = static_cast<const VulkanBuffer&>(buffer);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_BUFFER);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const IImage& image) const
{
	VkImage obj = static_cast<const VulkanImage&>(image);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_IMAGE);

	VkImageView objView = static_cast<const VulkanImage&>(image);
	SetName(name + "_View", reinterpret_cast<uint64_t>(objView), VK_OBJECT_TYPE_IMAGE_VIEW);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const IPipeline& pipeline) const
{
	VkPipeline obj = static_cast<const VulkanPipeline&>(pipeline);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_PIPELINE);

	VkPipelineLayout objLayout = static_cast<const VulkanPipeline&>(pipeline);
	SetName(name + "_Layout", reinterpret_cast<uint64_t>(objLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const ICommandBuffer& cmdBuf) const
{
	VkCommandBuffer obj = static_cast<const VulkanCommandBuffer&>(cmdBuf);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_COMMAND_BUFFER);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const ICommandPool& pool) const
{
	VkCommandPool obj = static_cast<const VulkanCommandPool&>(pool);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_COMMAND_POOL);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, const IShader& shader) const
{
	VkShaderModule obj = static_cast<const VulkanShader&>(shader);
	SetName(name, reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SHADER_MODULE);
}

void pompeii::vulkan::VulkanDebugUtils::SetName(const std::string& name, uint64_t objectHandle, VkObjectType objectType) const
{
	VkDebugUtilsObjectNameInfoEXT nameInfo{};
	nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
	nameInfo.objectType = objectType;
	nameInfo.objectHandle = objectHandle;
	nameInfo.pObjectName = name.c_str();

	PFN_SetDebugUtilsObjectNameEXT(m_Device, &nameInfo);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugUtils::AddValidationLayer(const char* layer)
{
	m_vValidationLayers.push_back(layer);
}
bool pompeii::vulkan::VulkanDebugUtils::CheckValidationLayerSupport() const
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

const std::vector<const char*>& pompeii::vulkan::VulkanDebugUtils::GetValidationLayers()
{
	return m_vValidationLayers;
}

//--------------------------------------------------
//    Helper
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugUtils::SetupMessenger()
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

void pompeii::vulkan::VulkanDebugUtils::SetupLabelling()
{
	PFN_CmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>
		(vkGetDeviceProcAddr(m_Device, "vkCmdBeginDebugUtilsLabelEXT"));
	PFN_CmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>
		(vkGetDeviceProcAddr(m_Device, "vkCmdEndDebugUtilsLabelEXT"));
	PFN_CmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>
		(vkGetDeviceProcAddr(m_Device, "vkCmdInsertDebugUtilsLabelEXT"));
}

void pompeii::vulkan::VulkanDebugUtils::SetupNames()
{
	PFN_SetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>
		(vkGetDeviceProcAddr(m_Device, "vkSetDebugUtilsObjectNameEXT"));
}

//--------------------------------------------------
//    Callback
//--------------------------------------------------
VkBool32 pompeii::vulkan::VulkanDebugUtils::MessageCallback(
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


//--------------------------------------------------
//    Static Helper
//--------------------------------------------------
void pompeii::vulkan::VulkanDebugUtils::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
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
