#ifndef DEVICE_H
#define DEVICE_H
#include "PhysicalDevice.h"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Device	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Device
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Device() = default;
		void Initialize(VkDevice device);
		void Destroy() const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDevice& GetDevice()		  const;
		const VkQueue&  GetGraphicQueue() const;
		const VkQueue&  GetPresentQueue() const;


		//--------------------------------------------------
		//    Device
		//--------------------------------------------------
		void WaitIdle() const;

	private:
		VkDevice	m_Device		{ VK_NULL_HANDLE };
		VkQueue		m_GraphicsQueue	{ VK_NULL_HANDLE };
		VkQueue		m_PresentQueue	{ VK_NULL_HANDLE };

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
		DeviceBuilder& SetFeatures(const VkPhysicalDeviceFeatures& features);
		Device& Build(const PhysicalDevice& physicalDevice, Device& device) const;

	private:
		VkPhysicalDeviceFeatures m_DesiredFeatures{};
	};
}
#endif // DEVICE_H