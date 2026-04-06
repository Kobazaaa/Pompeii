#ifndef POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H
#define POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IDebugMessenger.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>
#include <string>

namespace pompeii::vulkan
{
	struct DebugMessage
	{
		std::string severity;
		std::string type;
		std::string messageId;
		std::string message;
		std::vector<std::string> objects;
	};
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanDebugMessenger
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanDebugMessenger final : public IDebugMessenger
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanDebugMessenger(VkInstance instance);

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void Initialize() override;
		void Shutdown() override;


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		void AddValidationLayer(const char* layer);
		bool CheckValidationLayerSupport() const;
		const std::vector<const char*>& GetValidationLayers();

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	private:
		void SetupMessenger();

		static VkBool32 MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		                                VkDebugUtilsMessageTypeFlagsEXT messageType,
		                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		                                void* voidPtr);

		VkInstance m_Instance					{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_Messenger	{ VK_NULL_HANDLE };

		PFN_vkCreateDebugUtilsMessengerEXT	PFN_CreateDebugUtilsMessengerEXT	{ nullptr };
		PFN_vkDestroyDebugUtilsMessengerEXT PFN_DestroyDebugUtilsMessengerEXT	{ nullptr };

		std::vector<const char*> m_vValidationLayers{ };
	};
}

#endif // POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H
