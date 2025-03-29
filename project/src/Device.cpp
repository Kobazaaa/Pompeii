#include "Device.h"
#include <set>
#include <stdexcept>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Device	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Device::Device(VkDevice device)
	: m_Device(device)
{}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
VkDevice& pom::Device::GetDevice()		{ return m_Device; }
VkQueue& pom::Device::GetGraphicQueue() { return m_GraphicsQueue; }
VkQueue& pom::Device::GetPresentQueue() { return m_PresentQueue; }


//--------------------------------------------------
//    Device
//--------------------------------------------------
void pom::Device::WaitIdle() const { vkDeviceWaitIdle(m_Device); }





//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Device	Builder
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
pom::DeviceBuilder& pom::DeviceBuilder::SetFeatures(const VkPhysicalDeviceFeatures& features)
{
	m_DesiredFeatures = features;
	return *this;
}
pom::Device& pom::DeviceBuilder::Build(PhysicalDevice& physicalDevice, Device& device) const
{
	pom::QueueFamilyIndices indices = physicalDevice.GetQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	// The above line is done because Graphics and Present queue can be the same, doing this only passes the index once

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &m_DesiredFeatures;
	createInfo.enabledExtensionCount = physicalDevice.GetExtensionsCount();
	createInfo.ppEnabledExtensionNames = physicalDevice.GetExtensions().data();

	if (pom::Debugger::IsEnabled())
	{
		createInfo.enabledLayerCount = pom::Debugger::GetNumberOfLayers();
		createInfo.ppEnabledLayerNames = pom::Debugger::GetValidationLayers().data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice.GetPhysicalDevice(), &createInfo, nullptr, &device.GetDevice()) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Logical Device!");

	vkGetDeviceQueue(device.GetDevice(), indices.graphicsFamily.value(), 0, &device.GetGraphicQueue());
	vkGetDeviceQueue(device.GetDevice(), indices.presentFamily.value(), 0, &device.GetPresentQueue());

	return device;
}
