#ifndef POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H
#define POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "IDebugUtils.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>
#include <string>

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanDebugUtils
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanDebugUtils final : public IDebugUtils
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanDebugUtils(VkInstance instance, VkDevice device);

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void Initialize() override;
		void Shutdown() override;

		void BeginLabel(const ICommandBuffer& cmd, const std::string& name, const glm::vec4& color) override;
		void EndLabel(const ICommandBuffer& cmd) override;
		void InsertLabel(const ICommandBuffer& cmd, const std::string& name, const glm::vec4& color) override;

		void SetName(const std::string& name, const IBuffer& buffer) const override;
		void SetName(const std::string& name, const IImage& image) const override;
		void SetName(const std::string& name, const IPipeline& pipeline) const override;
		void SetName(const std::string& name, const ICommandBuffer& cmdBuf) const override;
		void SetName(const std::string& name, const ICommandPool& pool) const override;
		void SetName(const std::string& name, const IShader& shader) const override;
		void SetName(const std::string& name, uint64_t objectHandle, VkObjectType objectType) const;

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
		void SetupLabelling();
		void SetupNames();

		static VkBool32 MessageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		                                VkDebugUtilsMessageTypeFlagsEXT messageType,
		                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		                                void* voidPtr);

	private:
		VkInstance m_Instance					{ VK_NULL_HANDLE };
		VkDevice m_Device						{ VK_NULL_HANDLE };

		VkDebugUtilsMessengerEXT m_Messenger	{ VK_NULL_HANDLE };

		PFN_vkCreateDebugUtilsMessengerEXT	PFN_CreateDebugUtilsMessengerEXT	{ nullptr };
		PFN_vkDestroyDebugUtilsMessengerEXT PFN_DestroyDebugUtilsMessengerEXT	{ nullptr };

		PFN_vkSetDebugUtilsObjectNameEXT	PFN_SetDebugUtilsObjectNameEXT{ nullptr };
		PFN_vkCmdBeginDebugUtilsLabelEXT	PFN_CmdBeginDebugUtilsLabelEXT{ nullptr };
		PFN_vkCmdEndDebugUtilsLabelEXT		PFN_CmdEndDebugUtilsLabelEXT{ nullptr };
		PFN_vkCmdInsertDebugUtilsLabelEXT	PFN_CmdInsertDebugUtilsLabelEXT{ nullptr };

		std::vector<const char*> m_vValidationLayers{ };
	};
}

#endif // POMPEII_VULKAN_DEBUG_MESSENGER_IMPLEMENTATION_H
