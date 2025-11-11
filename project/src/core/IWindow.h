#ifndef POMPEII_INTERFACE_WINDOW_H
#define POMPEII_INTERFACE_WINDOW_H

// -- Standard Library --
#include <string>
#include <vector>

// -- Math Includes --
#include "glm/vec2.hpp"

// -- Vulkan Includes --
#include <vulkan/vulkan_core.h>

namespace pompeii
{
	class Instance;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Settings	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct WindowSettings
	{
		std::string title = "Default Window";
		int width = 600;
		int height = 800;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Window Interface	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class IWindow
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		virtual ~IWindow() = default;

		//--------------------------------------------------
		//    Lifecycle
		//--------------------------------------------------
		virtual void PollEvents()											= 0;
		virtual bool ShouldClose() const									= 0;
		virtual void Close()												= 0;

		//--------------------------------------------------
		//    Properties
		//--------------------------------------------------
		virtual void SetTitle(const std::string& title)						= 0;
		virtual float GetAspectRatio()							const		= 0;
		virtual glm::uvec2 GetFramebufferSize()					const		= 0;
		virtual bool IsFullScreen()								const		= 0;
		virtual void ToggleFullScreen()										= 0;

		virtual bool IsOutdated()								const		= 0;
		virtual void ResetOutdated()										= 0;

		virtual VkSurfaceKHR CreateVulkanSurface(const Instance& instance)	= 0;
		virtual std::vector<const char*> GetRequiredVulkanExtensions()const = 0;
		VkSurfaceKHR GetVulkanSurface()	const { return m_VulkanSurface; }

		//--------------------------------------------------
		//    Handle
		//--------------------------------------------------
		virtual void* GetNativeHandle()							const		= 0;

	protected:
		VkSurfaceKHR m_VulkanSurface{};
	};
}

#endif // POMPEII_INTERFACE_WINDOW_H