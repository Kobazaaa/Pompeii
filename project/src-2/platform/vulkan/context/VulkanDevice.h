#ifndef POMPEII_VULKAN_DEVICE_H
#define POMPEII_VULKAN_DEVICE_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declarations --
namespace pompeii::vulkan
{
	struct VulkanContext;
}


namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Device
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Device final
	{
	public:

		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Device() = default;
		~Device();

		Device(const Device& other) = delete;
		Device(Device&& other) noexcept = delete;
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other) noexcept = delete;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		[[nodiscard]] const VkQueue& GetGraphicQueue() const;
		[[nodiscard]] const VkQueue& GetPresentQueue() const;
		[[nodiscard]] const VkQueue& GetComputeQueue() const;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void WaitIdle() const;

		//--------------------------------------------------
		//    Operation Overloads
		//--------------------------------------------------
		operator VkDevice() const;


	private:
		VkDevice	m_Device		{ VK_NULL_HANDLE };
		VkQueue		m_GraphicsQueue	{ VK_NULL_HANDLE };
		VkQueue		m_PresentQueue	{ VK_NULL_HANDLE };
		VkQueue		m_ComputeQueue	{ VK_NULL_HANDLE };

		friend class DeviceBuilder;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Device	Builder
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class DeviceBuilder
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		DeviceBuilder() = default;

		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		DeviceBuilder& SetFeatures(const VkPhysicalDeviceFeatures2& features);
		void Build(VulkanContext& context) const;

	private:
		VkPhysicalDeviceFeatures2 m_DesiredFeatures{};
	};
}
#endif // POMPEII_VULKAN_DEVICE_H