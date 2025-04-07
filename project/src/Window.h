#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "Instance.h"

namespace pom
{
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
		GLFWwindow*		GetWindow()			const;
		glm::ivec2		GetSize()			const;
		float			GetAspectRatio()	const;

		bool			IsOutdated()		const;
		void			ResetOutdated();

		VkSurfaceKHR	GetVulkanSurface()	const;
		VkSurfaceKHR	CreateVulkanSurface(Instance& instance);
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
