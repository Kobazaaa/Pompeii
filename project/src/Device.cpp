// -- Standard Library --
#include <set>
#include <stdexcept>

// -- Pompeii Includes --
#include "Device.h"
#include "Debugger.h"
#include "Context.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Device	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pom::Device::Initialize(VkDevice device)	{ m_Device = device; }
void pom::Device::Destroy() const				{ vkDestroyDevice(m_Device, nullptr); }

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkDevice& pom::Device::GetHandle()		const { return m_Device; }
const VkQueue& pom::Device::GetGraphicQueue()	const { return m_GraphicsQueue; }
const VkQueue& pom::Device::GetPresentQueue()	const { return m_PresentQueue; }


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
void pom::DeviceBuilder::Build(Context& context) const
{
	pom::QueueFamilyIndices indices = context.physicalDevice.GetQueueFamilies();

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
	createInfo.enabledExtensionCount = context.physicalDevice.GetExtensionsCount();
	createInfo.ppEnabledExtensionNames = context.physicalDevice.GetExtensions().data();

	if (pom::Debugger::IsEnabled())
	{
		createInfo.enabledLayerCount = pom::Debugger::GetNumberOfLayers();
		createInfo.ppEnabledLayerNames = pom::Debugger::GetValidationLayers().data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(context.physicalDevice.GetHandle(), &createInfo, nullptr, &context.device.m_Device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Logical Device!");

	vkGetDeviceQueue(context.device.GetHandle(), indices.graphicsFamily.value(), 0, &context.device.m_GraphicsQueue);
	vkGetDeviceQueue(context.device.GetHandle(), indices.presentFamily.value(), 0, &context.device.m_PresentQueue);
}
