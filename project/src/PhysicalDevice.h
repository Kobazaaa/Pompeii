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

		bool IsComplete() const
		{
			return graphicsFamily.has_value()
				&& presentFamily.has_value();
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
		SwapChainSupportDetails			GetSwapChainSupportDetails(const VkSurfaceKHR surface);

		const std::vector<const char*>& GetExtensions()											const;
		uint32_t						GetExtensionsCount()									const;

		bool AreExtensionsSupported(const std::vector<const char*>& extensions)					const;

	private:
		friend class pom::PhysicalDeviceSelector;

		SwapChainSupportDetails QuerySwapChainSupport(VkSurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(const VkSurfaceKHR surface);


		VkPhysicalDevice			m_PhysicalDevice			{ VK_NULL_HANDLE };
		QueueFamilyIndices			m_QueueFamilyIndices		{};
		SwapChainSupportDetails		m_SwapChainSupportDetails	{};
		std::vector<const char*>	m_vExtensions				{};
		VkPhysicalDeviceProperties	m_Properties				{};
		VkPhysicalDeviceFeatures	m_Features					{};

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
		PhysicalDeviceSelector& PickPhysicalDevice(Context& context, VkSurfaceKHR surface);

	private:
		uint32_t RateDeviceSuitability(PhysicalDevice& device, VkSurfaceKHR surface) const;

		std::vector<const char*> m_vDesiredExtensions	{};
	};
}
#endif // PHYSICAL_DEVICE_H