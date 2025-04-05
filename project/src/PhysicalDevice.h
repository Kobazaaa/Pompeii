#ifndef PHYSICAL_DEVICE_H
#define PHYSICAL_DEVICE_H

#include <optional>
#include <vulkan/vulkan.h>
#include "Instance.h"

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
			return graphicsFamily.has_value() && presentFamily.has_value();
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
		explicit PhysicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions = {});

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkPhysicalDevice& GetPhysicalDevice();

		VkPhysicalDeviceProperties		GetProperties()							const;
		VkFormatProperties				GetFormatProperties(VkFormat format)	const;
		VkPhysicalDeviceFeatures		GetFeatures()							const;
		QueueFamilyIndices				GetQueueFamilies()						const;
		SwapChainSupportDetails			GetSwapChainSupportDetails()			const;

		const std::vector<const char*>& GetExtensions()							const;
		uint32_t						GetExtensionsCount()					const;

		bool AreExtensionsSupported(const std::vector<const char*>& extensions) const;

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
		PhysicalDeviceSelector& PickPhysicalDevice(Instance& instance, PhysicalDevice& physicalDevice, VkSurfaceKHR surface);

	private:
		uint32_t RateDeviceSuitability(PhysicalDevice& device, VkSurfaceKHR surface);

		std::vector<const char*> m_vDesiredExtensions	{};
	};
}
#endif // PHYSICAL_DEVICE_H