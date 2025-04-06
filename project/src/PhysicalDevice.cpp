#include "PhysicalDevice.h"

#include <iostream>
#include <map>
#include <set>
#include <vector>


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Physical Device	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::PhysicalDevice::Initialize(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions)
{
	m_PhysicalDevice = physicalDevice;
	m_vExtensions = extensions;

	if (physicalDevice == VK_NULL_HANDLE)
		return;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_Features);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkPhysicalDevice& pom::PhysicalDevice::GetPhysicalDevice()				const		{ return m_PhysicalDevice; }
VkPhysicalDeviceProperties pom::PhysicalDevice::GetProperties()					const		{ return m_Properties; }
VkFormatProperties pom::PhysicalDevice::GetFormatProperties(VkFormat format)	const		{ VkFormatProperties props{}; vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props); return props; }
VkPhysicalDeviceFeatures pom::PhysicalDevice::GetFeatures()						const		{ return m_Features; }

pom::QueueFamilyIndices pom::PhysicalDevice::GetQueueFamilies()					const		{ return m_QueueFamilyIndices; }
pom::SwapChainSupportDetails pom::PhysicalDevice::GetSwapChainSupportDetails()	const		{ return m_SwapChainSupportDetails; }

const std::vector<const char*>& pom::PhysicalDevice::GetExtensions()			const		{ return m_vExtensions; }
uint32_t pom::PhysicalDevice::GetExtensionsCount()								const		{ return static_cast<uint32_t>(m_vExtensions.size()); }

bool pom::PhysicalDevice::AreExtensionsSupported(const std::vector<const char*>& extensions) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(extensions.begin(), extensions.end());

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

//--------------------------------------------------
//    Queries -- PRIVATE
//--------------------------------------------------
pom::SwapChainSupportDetails pom::PhysicalDevice::QuerySwapChainSupport(const VkSurfaceKHR surface)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, surface, &m_SwapChainSupportDetails.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		m_SwapChainSupportDetails.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, surface, &formatCount, m_SwapChainSupportDetails.formats.data());
	}


	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		m_SwapChainSupportDetails.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, surface, &presentModeCount, m_SwapChainSupportDetails.presentModes.data());
	}

	return m_SwapChainSupportDetails;
}
pom::QueueFamilyIndices pom::PhysicalDevice::FindQueueFamilies(const VkSurfaceKHR surface)
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int index = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			m_QueueFamilyIndices.graphicsFamily = index;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, index, surface, &presentSupport);
		if (presentSupport)
			m_QueueFamilyIndices.presentFamily = index;

		if (m_QueueFamilyIndices.IsComplete())
			break;

		++index;
	}

	return m_QueueFamilyIndices;
}





//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Physical Device Selector	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Selector
//--------------------------------------------------
pom::PhysicalDeviceSelector& pom::PhysicalDeviceSelector::AddExtension(const char* ext)
{
	m_vDesiredExtensions.push_back(ext);
	return *this;
}
pom::PhysicalDeviceSelector& pom::PhysicalDeviceSelector::PickPhysicalDevice(const Instance& instance, PhysicalDevice& physicalDevice, VkSurfaceKHR surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance.GetInstance(), &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance.GetInstance(), &deviceCount, devices.data());

	std::multimap<uint32_t, PhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		PhysicalDevice physicalDev;
		physicalDev.Initialize(device, m_vDesiredExtensions);
		uint32_t score = RateDeviceSuitability(physicalDev, surface);
		candidates.insert(std::make_pair(score, physicalDev));
	}

	// Check if the best candidate is even suitable
	if (candidates.rbegin()->first > 0)
	{
		physicalDevice = candidates.rbegin()->second;
	}
	else
		throw std::runtime_error("Failed to find a suitable GPU!");

	if (Debugger::IsEnabled())
	{
		// Print selected GPU
		VkPhysicalDeviceProperties deviceProperties = physicalDevice.GetProperties();
		vkGetPhysicalDeviceProperties(physicalDevice.GetPhysicalDevice(), &deviceProperties);
		std::cout << "\nChosen GPU Data:\n"
			<< "\tName: " << deviceProperties.deviceName << "\n"
			<< "\tDriver Version: " << deviceProperties.driverVersion << "\n"
			<< "\tVendorID: " << deviceProperties.vendorID << "\n";
	}

	return *this;
}

uint32_t pom::PhysicalDeviceSelector::RateDeviceSuitability(PhysicalDevice& device, VkSurfaceKHR surface) const
{
	// Query Properties and Features
	const VkPhysicalDeviceProperties deviceProperties = device.GetProperties();
	const VkPhysicalDeviceFeatures deviceFeatures = device.GetFeatures();

	// Check specified extensions support
	if (!device.AreExtensionsSupported(m_vDesiredExtensions))
		return 0;

	// Check if the SwapChain is adequate
	SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport(surface);
	bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	if (!swapChainAdequate) return 0;

	// Check Queue Families Support
	QueueFamilyIndices indices = device.FindQueueFamilies(surface);
	if (!indices.IsComplete()) return 0;

	// Check if Anisotropy Sampling is available
	if (!deviceFeatures.samplerAnisotropy) return 0;

	// Calculate the Score
	uint32_t score = 0;

	// Dedicated/Discrete GPU have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	// Maximum possible size of textures affect quality
	score += deviceProperties.limits.maxImageDimension2D;

	return score;

}
