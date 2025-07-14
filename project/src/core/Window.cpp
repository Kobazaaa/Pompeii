// -- Standard Library --
#include <stdexcept>

// -- Pompeii Includes --
#include "Context.h"
#include "Window.h"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Window	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::Window::Window(const char* title, bool fullScreen, int width, int height)
{
	m_Fullscreen = fullScreen;
	m_WindowedSize = { width, height };

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (m_Fullscreen)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = mode->width;
		height = mode->height;
	}

	m_Size = { width, height };

	m_pWindow = glfwCreateWindow(width, height, title, m_Fullscreen ? monitor : nullptr, nullptr);
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, FrameBufferResizeCallback);
}
pompeii::Window::~Window()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}

//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
GLFWwindow* pompeii::Window::GetHandle()			const	{ return m_pWindow; }
glm::ivec2 pompeii::Window::GetSize()				const	{ return m_Size; }
float pompeii::Window::GetAspectRatio()				const	{ return static_cast<float>(m_Size.x) / static_cast<float>(m_Size.y); }
bool pompeii::Window::IsFullScreen()				const	{ return m_Fullscreen; }
void pompeii::Window::ToggleFullScreen()
{
	m_Fullscreen = !m_Fullscreen;

	if (!m_Fullscreen)
	{
		glfwSetWindowMonitor(
			m_pWindow, nullptr,
			100, 100,
			m_WindowedSize.x, m_WindowedSize.y,
			GLFW_DONT_CARE);
	}
	else
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwSetWindowMonitor(
			m_pWindow, glfwGetPrimaryMonitor(),
			0, 0,
			mode->width, mode->height,
			GLFW_DONT_CARE);
	}
}

bool pompeii::Window::IsOutdated()					const	{ return m_IsOutOfDate; }
void pompeii::Window::ResetOutdated()						{ m_IsOutOfDate = false; }
void pompeii::Window::WaitEvents()							{ glfwWaitEvents(); }

VkSurfaceKHR pompeii::Window::GetVulkanSurface()	const	{ return m_VulkanSurface;  }
VkSurfaceKHR pompeii::Window::CreateVulkanSurface(const Context& context)
{
	if (glfwCreateWindowSurface(context.instance.GetHandle(), m_pWindow, nullptr, &m_VulkanSurface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Window Surface!");
	return m_VulkanSurface;
}

// Callback
void pompeii::Window::FrameBufferResizeCallback(GLFWwindow* window, int, int)
{
	Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	pWindow->m_IsOutOfDate = true;
	glfwGetFramebufferSize(pWindow->m_pWindow, &pWindow->m_Size.x, &pWindow->m_Size.y);

	if (!pWindow->IsFullScreen())
	{
		glfwGetFramebufferSize(pWindow->m_pWindow, &pWindow->m_WindowedSize.x, &pWindow->m_WindowedSize.y);
	}
}
