#ifndef PHYSICAL_DEVICE_H
#define PHYSICAL_DEVICE_H

// -- Vulkan Includes
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <optional>
#include <vector>

// -- Forward Declarations --
namespace pom { struct Context; }


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Helper Structs	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		std::optional<uint32_t> computeFamily;

		bool IsComplete() const
		{
			return graphicsFamily.has_value()
				&& presentFamily.has_value()
				&& computeFamily.has_value();
		}
	};


	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Physical Device	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class PhysicalDeviceSelector;
	class PhysicalDevice
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		PhysicalDevice() = default;
		void Initialize(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions = {});

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkPhysicalDevice&			GetHandle()												const;

		VkPhysicalDeviceProperties		GetProperties()											const;
		VkFormatProperties				GetFormatProperties(VkFormat format)					const;
		VkPhysicalDeviceFeatures		GetFeatures()											const;
		QueueFamilyIndices				GetQueueFamilies()										const;
		VkSampleCountFlagBits 			GetMaxSampleCount()										const;
		SwapChainSupportDetails			GetSwapChainSupportDetails(const VkSurfaceKHR surface);

		const std::vector<const char*>& GetExtensions()											const;
		uint32_t						GetExtensionsCount()									const;

		bool AreExtensionsSupported(const std::vector<const char*>& extensions)					const;
		bool AreFeaturesSupported(const VkPhysicalDeviceFeatures2& features)					const;

		bool CheckFeatures(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& available) const;
		bool CheckFeatures(const VkPhysicalDeviceVulkan11Features& requested, const VkPhysicalDeviceVulkan11Features& available) const;
		bool CheckFeatures(const VkPhysicalDeviceVulkan12Features& requested, const VkPhysicalDeviceVulkan12Features& available) const;
		bool CheckFeatures(const VkPhysicalDeviceVulkan13Features& requested, const VkPhysicalDeviceVulkan13Features& available) const;

	private:
		friend class PhysicalDeviceSelector;

		SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(const VkSurfaceKHR surface);


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
		void PickPhysicalDevice(Context& context, VkSurfaceKHR surface) const;

	private:
		uint32_t RateDeviceSuitability(PhysicalDevice& device, VkSurfaceKHR surface) const;

		VkPhysicalDeviceFeatures2 m_RequestedFeatures;
		std::vector<const char*> m_vDesiredExtensions	{};
	};
}
#endif // PHYSICAL_DEVICE_H