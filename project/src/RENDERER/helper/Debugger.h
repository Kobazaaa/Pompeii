#ifndef DEBUGGER_H
#define DEBUGGER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <string>
#include <vector>

// -- Math Includes --
#include "glm/vec4.hpp"

// -- Forward Declarations --
namespace pompeii
{
	class CommandBuffer;
	struct Context;
}


namespace pompeii
{
	struct InstanceDebugUtils
	{
		VkInstance							instance						{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT			debugUtilsMessenger				{ VK_NULL_HANDLE };

		PFN_vkCreateDebugUtilsMessengerEXT	createDebugUtilsMessengerEXT	{ nullptr };
		PFN_vkDestroyDebugUtilsMessengerEXT destroyDebugUtilsMessengerEXT	{ nullptr };
	};
	struct DeviceDebugUtils
	{
		VkDevice							device							{ VK_NULL_HANDLE };

		PFN_vkSetDebugUtilsObjectNameEXT	setDebugUtilsObjectNameEXT		{ nullptr };
		PFN_vkCmdBeginDebugUtilsLabelEXT	cmdBeginDebugUtilsLabelEXT		{ nullptr };
		PFN_vkCmdEndDebugUtilsLabelEXT		cmdEndDebugUtilsLabelEXT		{ nullptr };
		PFN_vkCmdInsertDebugUtilsLabelEXT	cmdInsertDebugUtilsLabelEXT		{ nullptr };
	};

	class Debugger final
	{
	public:
		//--------------------------------------------------
		//    Setup
		//--------------------------------------------------
		static void SetEnabled(bool enabled);
		static void AddValidationLayer(const char* layer);
		static void Setup(const Context& context);
		static void Destroy();

		static void SetDebugObjectName(uint64_t objectHandle, VkObjectType objectType, const std::string& name);
		static void BeginDebugLabel(CommandBuffer & cmdBuffer, const std::string& name, const glm::vec4& color);
		static void InsertDebugLabel(CommandBuffer& cmdBuffer, const std::string& name, const glm::vec4& color);
		static void EndDebugLabel(CommandBuffer& cmdBuffer);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		static bool CheckValidationLayerSupport();

		static bool IsEnabled();

		static const std::vector<const char*>& GetValidationLayers();
		static uint32_t GetNumberOfLayers();

	private:
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
															void* pUserData);

		static void SetupInstanceDebugUtils(const Context& context);
		static void SetupDeviceDebugUtils(const Context& context);

		inline static bool							m_IsEnabled				{ false };
		inline static std::vector<const char*>		m_vValidationLayers		{ };

		inline static InstanceDebugUtils			m_InstanceDebugUtils	{ };
		inline static DeviceDebugUtils				m_DeviceDebugUtils		{ };
	};
}

#endif // DEBUGGER_H