#ifndef DEVICE_H
#define DEVICE_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declarations --
namespace pom { struct Context; }


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
		explicit Device() = default;
		~Device() = default;
		Device(const Device& other) = delete;
		Device(Device&& other) noexcept = delete;
		Device& operator=(const Device& other) = delete;
		Device& operator=(Device&& other) noexcept = delete;

		void Initialize(VkDevice device);
		void Destroy() const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkDevice& GetHandle()		  const;
		const VkQueue&  GetGraphicQueue() const;
		const VkQueue&  GetPresentQueue() const;
		const VkQueue&  GetComputeQueue() const;


		//--------------------------------------------------
		//    Device
		//--------------------------------------------------
		void WaitIdle() const;

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
		void Build(Context& context) const;

	private:
		VkPhysicalDeviceFeatures2 m_DesiredFeatures{};
	};
}
#endif // DEVICE_H