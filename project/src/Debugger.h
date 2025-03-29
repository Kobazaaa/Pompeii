#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace pom
{
	class Instance;

	class Debugger final
	{
	public:
		//--------------------------------------------------
		//    Setup
		//--------------------------------------------------
		static void SetEnabled(bool enabled);
		static void AddValidationLayer(const char* layer);
		static void SetupMessenger(Instance& instance);
		static void DestroyMessenger(Instance& instance);

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

		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
												const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

		inline static bool							m_IsEnabled			{ false };
		inline static std::vector<const char*>		m_vValidationLayers	{};
		inline static VkDebugUtilsMessengerEXT		m_DebugMessenger	{};
	};
}

#endif // DEBUGGER_H