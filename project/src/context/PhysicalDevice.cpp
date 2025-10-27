// -- Standard Library --
#include <iostream>
#include <map>
#include <set>
#include <vector>

// -- Pompeii Includes --
#include "PhysicalDevice.h"
#include "Debugger.h"
#include "Context.h"
#include "ConsoleTextSettings.h"



//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Physical Device	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::PhysicalDevice::Initialize(VkPhysicalDevice physicalDevice, const std::vector<const char*>& extensions)
{
	m_PhysicalDevice = physicalDevice;
	m_vExtensions = extensions;

	if (physicalDevice == VK_NULL_HANDLE)
		return;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_Properties);
	vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &m_Features);
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
const VkPhysicalDevice& pompeii::PhysicalDevice::GetHandle()						const		{ return m_PhysicalDevice; }
VkPhysicalDeviceProperties pompeii::PhysicalDevice::GetProperties()					const		{ return m_Properties; }
VkFormatProperties pompeii::PhysicalDevice::GetFormatProperties(VkFormat format)	const		{ VkFormatProperties props{}; vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props); return props; }
VkPhysicalDeviceFeatures pompeii::PhysicalDevice::GetFeatures()						const		{ return m_Features.features; }
pompeii::QueueFamilyIndices pompeii::PhysicalDevice::GetQueueFamilies()					const		{ return m_QueueFamilyIndices; }
VkSampleCountFlagBits pompeii::PhysicalDevice::GetMaxSampleCount()					const
{
	VkPhysicalDeviceProperties properties = GetProperties();

	const VkSampleCountFlags counts = properties.limits.framebufferColorSampleCounts
									& properties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT)
		return VK_SAMPLE_COUNT_64_BIT;
	if (counts & VK_SAMPLE_COUNT_32_BIT)
		return VK_SAMPLE_COUNT_32_BIT;
	if (counts & VK_SAMPLE_COUNT_16_BIT)
		return VK_SAMPLE_COUNT_16_BIT;
	if (counts & VK_SAMPLE_COUNT_8_BIT)
		return VK_SAMPLE_COUNT_8_BIT;
	if (counts & VK_SAMPLE_COUNT_4_BIT)
		return VK_SAMPLE_COUNT_4_BIT;
	if (counts & VK_SAMPLE_COUNT_2_BIT)
		return VK_SAMPLE_COUNT_2_BIT;

	return VK_SAMPLE_COUNT_1_BIT;
}
pompeii::SwapChainSupportDetails pompeii::PhysicalDevice::GetSwapChainSupportDetails(const VkSurfaceKHR surface)
{
	QuerySwapChainSupport(surface);
	return m_SwapChainSupportDetails;
}

const std::vector<const char*>& pompeii::PhysicalDevice::GetExtensions()			const		{ return m_vExtensions; }
uint32_t pompeii::PhysicalDevice::GetExtensionsCount()								const		{ return static_cast<uint32_t>(m_vExtensions.size()); }

bool pompeii::PhysicalDevice::AreExtensionsSupported(const std::vector<const char*>& extensions) const
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
bool pompeii::PhysicalDevice::AreFeaturesSupported(const VkPhysicalDeviceFeatures2& features) const
{
	VkPhysicalDeviceVulkan13Features requested13;
	VkPhysicalDeviceVulkan12Features requested12;
	VkPhysicalDeviceVulkan11Features requested11;
	VkPhysicalDeviceFeatures requested = features.features;

	VkBaseOutStructure* current = static_cast<VkBaseOutStructure*>(features.pNext);
	while (current)
	{
		if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES)
			requested11 = *reinterpret_cast<VkPhysicalDeviceVulkan11Features*>(current);
		else if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES)
			requested12 = *reinterpret_cast<VkPhysicalDeviceVulkan12Features*>(current);
		else if (current->sType == VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES)
			requested13 = *reinterpret_cast<VkPhysicalDeviceVulkan13Features*>(current);
		current = current->pNext;
	}

	bool support = true;
	support &= CheckFeatures(requested, m_Features.features);
	support &= CheckFeatures(requested11, m_Features11);
	support &= CheckFeatures(requested12, m_Features12);
	support &= CheckFeatures(requested13, m_Features13);
	return support;
}

#define CHECK_FEATURE(feature) \
		if (requested.feature == VK_TRUE && available.feature == VK_FALSE) { \
		    std::cout << WARNING_TXT << m_Properties.deviceName << " is missing feature: " << #feature << ". Looking for other suitable GPU!\n" << RESET_TXT; \
		    return false; \
		}
bool pompeii::PhysicalDevice::CheckFeatures(const VkPhysicalDeviceFeatures& requested, const VkPhysicalDeviceFeatures& available) const
{
	CHECK_FEATURE(robustBufferAccess)
	CHECK_FEATURE(fullDrawIndexUint32)
	CHECK_FEATURE(imageCubeArray)
	CHECK_FEATURE(independentBlend)
	CHECK_FEATURE(geometryShader)
	CHECK_FEATURE(tessellationShader)
	CHECK_FEATURE(sampleRateShading)
	CHECK_FEATURE(dualSrcBlend)
	CHECK_FEATURE(logicOp)
	CHECK_FEATURE(multiDrawIndirect)
	CHECK_FEATURE(drawIndirectFirstInstance)
	CHECK_FEATURE(depthClamp)
	CHECK_FEATURE(depthBiasClamp)
	CHECK_FEATURE(fillModeNonSolid)
	CHECK_FEATURE(depthBounds)
	CHECK_FEATURE(wideLines)
	CHECK_FEATURE(largePoints)
	CHECK_FEATURE(alphaToOne)
	CHECK_FEATURE(multiViewport)
	CHECK_FEATURE(samplerAnisotropy)
	CHECK_FEATURE(textureCompressionETC2)
	CHECK_FEATURE(textureCompressionASTC_LDR)
	CHECK_FEATURE(textureCompressionBC)
	CHECK_FEATURE(occlusionQueryPrecise)
	CHECK_FEATURE(pipelineStatisticsQuery)
	CHECK_FEATURE(vertexPipelineStoresAndAtomics)
	CHECK_FEATURE(fragmentStoresAndAtomics)
	CHECK_FEATURE(shaderTessellationAndGeometryPointSize)
	CHECK_FEATURE(shaderImageGatherExtended)
	CHECK_FEATURE(shaderStorageImageMultisample)
	CHECK_FEATURE(shaderStorageImageReadWithoutFormat)
	CHECK_FEATURE(shaderStorageImageWriteWithoutFormat)
	CHECK_FEATURE(shaderUniformBufferArrayDynamicIndexing)
	CHECK_FEATURE(shaderSampledImageArrayDynamicIndexing)
	CHECK_FEATURE(shaderStorageBufferArrayDynamicIndexing)
	CHECK_FEATURE(shaderStorageImageArrayDynamicIndexing)
	CHECK_FEATURE(shaderClipDistance)
	CHECK_FEATURE(shaderFloat64)
	CHECK_FEATURE(shaderInt64)
	CHECK_FEATURE(shaderInt16)
	CHECK_FEATURE(shaderResourceResidency)
	CHECK_FEATURE(shaderResourceMinLod)
	CHECK_FEATURE(sparseBinding)
	CHECK_FEATURE(sparseResidencyBuffer)
	CHECK_FEATURE(sparseResidencyImage2D)
	CHECK_FEATURE(sparseResidencyImage3D)
	CHECK_FEATURE(sparseResidency2Samples)
	CHECK_FEATURE(sparseResidency4Samples)
	CHECK_FEATURE(sparseResidency8Samples)
	CHECK_FEATURE(sparseResidency16Samples)
	CHECK_FEATURE(sparseResidencyAliased)
	CHECK_FEATURE(variableMultisampleRate)
	CHECK_FEATURE(inheritedQueries)
	return true;
}
bool pompeii::PhysicalDevice::CheckFeatures(const VkPhysicalDeviceVulkan11Features& requested, const VkPhysicalDeviceVulkan11Features& available) const
{
	CHECK_FEATURE(storageBuffer16BitAccess)
	CHECK_FEATURE(uniformAndStorageBuffer16BitAccess)
	CHECK_FEATURE(storagePushConstant16)
	CHECK_FEATURE(storageInputOutput16)
	CHECK_FEATURE(multiview)
	CHECK_FEATURE(multiviewGeometryShader)
	CHECK_FEATURE(multiviewTessellationShader)
	CHECK_FEATURE(variablePointersStorageBuffer)
	CHECK_FEATURE(variablePointers)
	CHECK_FEATURE(protectedMemory)
	CHECK_FEATURE(samplerYcbcrConversion)
	CHECK_FEATURE(shaderDrawParameters)
	return true;
}
bool pompeii::PhysicalDevice::CheckFeatures(const VkPhysicalDeviceVulkan12Features& requested, const VkPhysicalDeviceVulkan12Features& available) const
{
	CHECK_FEATURE(samplerMirrorClampToEdge)
	CHECK_FEATURE(drawIndirectCount)
	CHECK_FEATURE(storageBuffer8BitAccess)
	CHECK_FEATURE(uniformAndStorageBuffer8BitAccess)
	CHECK_FEATURE(storagePushConstant8)
	CHECK_FEATURE(shaderBufferInt64Atomics)
	CHECK_FEATURE(shaderSharedInt64Atomics)
	CHECK_FEATURE(shaderFloat16)
	CHECK_FEATURE(shaderInt8)
	CHECK_FEATURE(descriptorIndexing)
	CHECK_FEATURE(shaderInputAttachmentArrayDynamicIndexing)
	CHECK_FEATURE(shaderUniformTexelBufferArrayDynamicIndexing)
	CHECK_FEATURE(shaderStorageTexelBufferArrayDynamicIndexing)
	CHECK_FEATURE(shaderUniformBufferArrayNonUniformIndexing)
	CHECK_FEATURE(shaderSampledImageArrayNonUniformIndexing)
	CHECK_FEATURE(shaderStorageBufferArrayNonUniformIndexing)
	CHECK_FEATURE(shaderStorageImageArrayNonUniformIndexing)
	CHECK_FEATURE(shaderInputAttachmentArrayNonUniformIndexing)
	CHECK_FEATURE(shaderUniformTexelBufferArrayNonUniformIndexing)
	CHECK_FEATURE(shaderStorageTexelBufferArrayNonUniformIndexing)
	CHECK_FEATURE(descriptorBindingUniformBufferUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingSampledImageUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingStorageImageUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingStorageBufferUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingUniformTexelBufferUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingStorageTexelBufferUpdateAfterBind)
	CHECK_FEATURE(descriptorBindingUpdateUnusedWhilePending)
	CHECK_FEATURE(descriptorBindingPartiallyBound)
	CHECK_FEATURE(descriptorBindingVariableDescriptorCount)
	CHECK_FEATURE(runtimeDescriptorArray)
	CHECK_FEATURE(samplerFilterMinmax)
	CHECK_FEATURE(scalarBlockLayout)
	CHECK_FEATURE(imagelessFramebuffer)
	CHECK_FEATURE(uniformBufferStandardLayout)
	CHECK_FEATURE(shaderSubgroupExtendedTypes)
	CHECK_FEATURE(separateDepthStencilLayouts)
	CHECK_FEATURE(hostQueryReset)
	CHECK_FEATURE(timelineSemaphore)
	CHECK_FEATURE(bufferDeviceAddress)
	CHECK_FEATURE(bufferDeviceAddressCaptureReplay)
	CHECK_FEATURE(bufferDeviceAddressMultiDevice)
	CHECK_FEATURE(vulkanMemoryModel)
	CHECK_FEATURE(vulkanMemoryModelDeviceScope)
	CHECK_FEATURE(vulkanMemoryModelAvailabilityVisibilityChains)
	CHECK_FEATURE(shaderOutputViewportIndex)
	CHECK_FEATURE(shaderOutputLayer)
	CHECK_FEATURE(subgroupBroadcastDynamicId)
	return true;
}
bool pompeii::PhysicalDevice::CheckFeatures(const VkPhysicalDeviceVulkan13Features& requested, const VkPhysicalDeviceVulkan13Features& available) const
{
	CHECK_FEATURE(robustImageAccess)
	CHECK_FEATURE(inlineUniformBlock)
	CHECK_FEATURE(descriptorBindingInlineUniformBlockUpdateAfterBind)
	CHECK_FEATURE(pipelineCreationCacheControl)
	CHECK_FEATURE(privateData)
	CHECK_FEATURE(shaderDemoteToHelperInvocation)
	CHECK_FEATURE(shaderTerminateInvocation)
	CHECK_FEATURE(subgroupSizeControl)
	CHECK_FEATURE(computeFullSubgroups)
	CHECK_FEATURE(synchronization2)
	CHECK_FEATURE(textureCompressionASTC_HDR)
	CHECK_FEATURE(shaderZeroInitializeWorkgroupMemory)
	CHECK_FEATURE(dynamicRendering)
	CHECK_FEATURE(shaderIntegerDotProduct)
	CHECK_FEATURE(maintenance4)
	return true;
}
#undef CHECK_FEATURE


//--------------------------------------------------
//    Queries -- PRIVATE
//--------------------------------------------------
pompeii::SwapChainSupportDetails pompeii::PhysicalDevice::QuerySwapChainSupport(const VkSurfaceKHR surface)
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
pompeii::QueueFamilyIndices pompeii::PhysicalDevice::FindQueueFamilies()
{
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	int index = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// This commented code looks for separate Graphics and Compute queues (which might still be the same). But in a scenario where they aren't
		// This would cause the rest of the code to stop working, as I am submitting compute commands into a graphics queue, since as of now, they are both recorded in the same
		// command buffer for ease of use. I think ideally this wouldn't be the case, and is potentially an improvement waiting to happen :)
		// For now, the fix I am imposing is just enforcing the graphics and compute queue to be the same, as can be seen further below.
		//if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		//	m_QueueFamilyIndices.graphicsFamily = index;
		//if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		//	m_QueueFamilyIndices.computeFamily = index;

		// Look for shared Graphics and Compute Queue
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			m_QueueFamilyIndices.graphicsFamily = index;
			m_QueueFamilyIndices.computeFamily = index;
		}

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
pompeii::PhysicalDeviceSelector& pompeii::PhysicalDeviceSelector::AddExtension(const char* ext)
{
	m_vDesiredExtensions.push_back(ext);
	return *this;
}
pompeii::PhysicalDeviceSelector& pompeii::PhysicalDeviceSelector::CheckForFeatures(const VkPhysicalDeviceFeatures2& features)
{
	m_RequestedFeatures = features;
	return *this;
}
void pompeii::PhysicalDeviceSelector::PickPhysicalDevice(Context& context) const
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(context.instance.GetHandle(), &deviceCount, nullptr);

	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(context.instance.GetHandle(), &deviceCount, devices.data());

	std::multimap<uint32_t, PhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		PhysicalDevice physicalDev;
		physicalDev.Initialize(device, m_vDesiredExtensions);
		uint32_t score = RateDeviceSuitability(physicalDev);
		candidates.insert(std::make_pair(score, physicalDev));
	}

	// Check if the best candidate is even suitable
	if (candidates.rbegin()->first > 0)
	{
		context.physicalDevice = candidates.rbegin()->second;
	}
	else
		throw std::runtime_error("Failed to find a suitable GPU!");

	if (Debugger::IsEnabled())
	{
		// Print selected GPU
		VkPhysicalDeviceProperties deviceProperties = context.physicalDevice.GetProperties();
		vkGetPhysicalDeviceProperties(context.physicalDevice.GetHandle(), &deviceProperties);
		std::cout << INFO_TXT << "Chosen GPU Data:\n"
			<< "\tName: " << deviceProperties.deviceName << "\n"
			<< "\tDriver Version: " << deviceProperties.driverVersion << "\n"
			<< "\tVendorID: " << deviceProperties.vendorID << "\n\n" << RESET_TXT;
	}
}

uint32_t pompeii::PhysicalDeviceSelector::RateDeviceSuitability(PhysicalDevice& device) const
{
	// Query Properties and Features
	const VkPhysicalDeviceProperties deviceProperties = device.GetProperties();
	const VkPhysicalDeviceFeatures deviceFeatures = device.GetFeatures();

	// Check specified extensions support
	if (!device.AreExtensionsSupported(m_vDesiredExtensions))
		return 0;

	// Check specified feature support
	if (!device.AreFeaturesSupported(m_RequestedFeatures))
		return 0;

	// Check if the SwapChain is adequate
	//SwapChainSupportDetails swapChainSupport = device.QuerySwapChainSupport(surface);
	//bool swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	//if (!swapChainAdequate) return 0;

	// Check Queue Families Support
	QueueFamilyIndices indices = device.FindQueueFamilies();
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
