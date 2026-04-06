// -- Standard Library --
#include <set>
#include <stdexcept>

// -- Pompeii Includes --
#include "Device.h"
#include "VulkanContext.h"
#include "VulkanDebugMessenger.h"

using namespace pompeii::vulkan;

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Device
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
Device::~Device()
{
	if (m_Device)
		vkDestroyDevice(m_Device, nullptr);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkQueue& Device::GetGraphicQueue()	const { return m_GraphicsQueue; }
const VkQueue& Device::GetPresentQueue()	const { return m_PresentQueue; }
const VkQueue& Device::GetComputeQueue()	const { return m_ComputeQueue; }

//--------------------------------------------------
//    Functionality
//--------------------------------------------------
void Device::WaitIdle() const { vkDeviceWaitIdle(m_Device); }

//--------------------------------------------------
//    Operation Overloads
//--------------------------------------------------
Device::operator VkDevice () const { return m_Device; }



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Device	Builder
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Builder
//--------------------------------------------------
DeviceBuilder& DeviceBuilder::SetFeatures(const VkPhysicalDeviceFeatures2& features)
{
	m_DesiredFeatures = features;
	return *this;
}
void DeviceBuilder::Build(VulkanContext& context, VulkanDebugMessenger* pDebugMessenger) const
{
	QueueFamilyIndices indices = context.physicalDevice.GetQueueFamilies();

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value(), indices.computeFamily.value() };
	// The above line is done because Graphics, Present, or Compute queue can be the same, doing this only passes the index once

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
	createInfo.pNext = &m_DesiredFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = nullptr;
	createInfo.enabledExtensionCount = context.physicalDevice.GetExtensionsCount();
	createInfo.ppEnabledExtensionNames = context.physicalDevice.GetExtensions().data();

	if (pDebugMessenger)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(pDebugMessenger->GetValidationLayers().size());
		createInfo.ppEnabledLayerNames = pDebugMessenger->GetValidationLayers().data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(context.physicalDevice, &createInfo, nullptr, &context.device.m_Device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Logical Device!");

	vkGetDeviceQueue(context.device, indices.graphicsFamily.value(), 0, &context.device.m_GraphicsQueue);
	vkGetDeviceQueue(context.device, indices.presentFamily.value(), 0, &context.device.m_PresentQueue);
	vkGetDeviceQueue(context.device, indices.computeFamily.value(), 0, &context.device.m_ComputeQueue);
}

