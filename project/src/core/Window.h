#ifndef WINDOW_H
#define WINDOW_H

// -- Vulkan Includes --
#include "vulkan/vulkan.h"

// -- GLFW Includes --
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// -- Math Includes --
#include <glm/glm.hpp>

//-- Forward Declarations --
namespace pompeii { struct Context; }

// -- Class --
namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Window	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Window final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Window(const char* title, bool fullScreen, int width = 800, int height = 600);
		~Window();
		Window(const Window& other) = delete;
		Window(Window&& other) noexcept = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) noexcept = delete;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		GLFWwindow*		GetHandle()			const;
		glm::ivec2		GetSize()			const;
		float			GetAspectRatio()	const;
		bool			IsFullScreen()		const;
		void			ToggleFullScreen();

		bool			IsOutdated()		const;
		void			ResetOutdated();
		static void		WaitEvents();

		VkSurfaceKHR	GetVulkanSurface()	const;
		VkSurfaceKHR	CreateVulkanSurface(const Context& context);
	private:
		// Callback for when the window gets resized
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow*		m_pWindow			{ nullptr };
		glm::ivec2		m_Size				{ };
		glm::ivec2		m_WindowedSize		{ };
		bool			m_Fullscreen		{ };
		bool			m_IsOutOfDate		{ false };
		VkSurfaceKHR	m_VulkanSurface		{ VK_NULL_HANDLE };
	};
}
#endif // WINDOW_H
