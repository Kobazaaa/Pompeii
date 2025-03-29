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
		explicit Device(VkDevice device);


		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		VkDevice& GetDevice();
		VkQueue& GetGraphicQueue();
		VkQueue& GetPresentQueue();


		//--------------------------------------------------
		//    Device
		//--------------------------------------------------
		void WaitIdle() const;

	private:
		VkDevice	m_Device		{ VK_NULL_HANDLE };
		VkQueue		m_GraphicsQueue	{ VK_NULL_HANDLE };
		VkQueue		m_PresentQueue	{ VK_NULL_HANDLE };
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
		Device& Build(PhysicalDevice& physicalDevice, Device& device) const;

	private:
		VkPhysicalDeviceFeatures m_DesiredFeatures{};
	};
}
#endif // DEVICE_H