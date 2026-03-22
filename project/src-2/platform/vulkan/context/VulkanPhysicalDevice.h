#ifndef POMPEII_VULKAN_PHYSICAL_DEVICE_H
#define POMPEII_VULKAN_PHYSICAL_DEVICE_H

// -- Vulkan Includes
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <optional>
#include <vector>

// -- Forward Declarations --
namespace pompeii::vulkan
{
	struct VulkanContext;
}


namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Helper Structs
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct SwapChainSupportDetails
	{
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
		VkSurfaceCapabilitiesKHR capabilities;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;

		[[nodiscard]] bool IsComplete() const
		{
			return graphicsFamily.has_value()
				&& presentFamily.has_value()
				&& computeFamily.has_value();
		}
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Physical Device
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class PhysicalDevice
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit PhysicalDevice() = default;
		explicit PhysicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions = {});

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] VkPhysicalDeviceProperties	GetProperties()											const;
		[[nodiscard]] VkFormatProperties			GetFormatProperties(VkFormat format)					const;
		[[nodiscard]] VkPhysicalDeviceFeatures		GetFeatures()											const;
		[[nodiscard]] QueueFamilyIndices			GetQueueFamilies()										const;
		[[nodiscard]] VkSampleCountFlagBits			GetMaxSampleCount()										const;
		[[nodiscard]] SwapChainSupportDetails		GetSwapChainSupportDetails(const VkSurfaceKHR surface);

		[[nodiscard]] const std::vector<const char*>& GetExtensions()										const;
		[[nodiscard]] uint32_t						GetExtensionsCount()									const;

		[[nodiscard]] bool AreExtensionsSupported(const std::vector<const char*>& extensions)				const;
		[[nodiscard]] bool AreFeaturesSupported(const VkPhysicalDeviceFeatures2& features)					const;

		[[nodiscard]] bool CheckFeatures(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& available) const;
		[[nodiscard]] bool CheckFeatures(const VkPhysicalDeviceVulkan11Features& requested, const VkPhysicalDeviceVulkan11Features& available) const;
		[[nodiscard]] bool CheckFeatures(const VkPhysicalDeviceVulkan12Features& requested, const VkPhysicalDeviceVulkan12Features& available) const;
		[[nodiscard]] bool CheckFeatures(const VkPhysicalDeviceVulkan13Features& requested, const VkPhysicalDeviceVulkan13Features& available) const;

		//--------------------------------------------------
		//    Operation Overloads
		//--------------------------------------------------
		operator VkPhysicalDevice() const;

	private:
		SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(VkSurfaceKHR surface);


		VkPhysicalDevice				 m_PhysicalDevice			{ VK_NULL_HANDLE };
		QueueFamilyIndices				 m_QueueFamilyIndices		{};
		SwapChainSupportDetails			 m_SwapChainSupportDetails	{};
		std::vector<const char*>		 m_vExtensions				{};
		VkPhysicalDeviceProperties		 m_Properties				{};

		VkPhysicalDeviceVulkan13Features m_Features13				{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = nullptr };
		VkPhysicalDeviceVulkan12Features m_Features12				{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &m_Features13 };
		VkPhysicalDeviceVulkan11Features m_Features11				{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, .pNext = &m_Features12 };
		VkPhysicalDeviceFeatures2		 m_Features					{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,		  .pNext = &m_Features11 };

		friend class PhysicalDeviceSelector;
	};



	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Physical Device Selector
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class PhysicalDeviceSelector
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		PhysicalDeviceSelector() = default;

		//--------------------------------------------------
		//    Selector
		//--------------------------------------------------
		PhysicalDeviceSelector& AddExtension(const char* ext);
		PhysicalDeviceSelector& CheckForFeatures(const VkPhysicalDeviceFeatures2& features);
		void PickPhysicalDevice(VulkanContext& context) const;

	private:
		uint32_t RateDeviceSuitability(PhysicalDevice& device, VkSurfaceKHR surface) const;

		VkPhysicalDeviceFeatures2 m_RequestedFeatures;
		std::vector<const char*> m_vDesiredExtensions	{};
	};
}
#endif // POMPEII_VULKAN_PHYSICAL_DEVICE_H