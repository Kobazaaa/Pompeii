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
namespace pom { struct Context; }

// -- Class --
namespace pom
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
		Window() = default;
		void Initialize(int width, int height, const char* title);
		void Destroy() const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		GLFWwindow*		GetHandle()			const;
		glm::ivec2		GetSize()			const;
		float			GetAspectRatio()	const;

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
		bool			m_IsOutOfDate		{ false };
		VkSurfaceKHR	m_VulkanSurface		{ VK_NULL_HANDLE };
	};
}
#endif // WINDOW_H
