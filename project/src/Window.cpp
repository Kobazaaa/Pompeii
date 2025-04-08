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
void pom::Window::Initialize(int width, int height, const char* title)
{
	m_Size = { width, height };

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, FrameBufferResizeCallback);
}
void pom::Window::Destroy() const
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
GLFWwindow* pom::Window::GetHandle()			const	{ return m_pWindow; }
glm::ivec2 pom::Window::GetSize()				const	{ return m_Size; }
float pom::Window::GetAspectRatio()				const	{ return static_cast<float>(m_Size.x) / static_cast<float>(m_Size.y); }

bool pom::Window::IsOutdated()					const	{ return m_IsOutOfDate; }
void pom::Window::ResetOutdated()						{ m_IsOutOfDate = false; }
void pom::Window::WaitEvents()							{ glfwWaitEvents(); }

VkSurfaceKHR pom::Window::GetVulkanSurface()	const	{ return m_VulkanSurface;  }
VkSurfaceKHR pom::Window::CreateVulkanSurface(const Context& context)
{
	if (glfwCreateWindowSurface(context.instance.GetHandle(), m_pWindow, nullptr, &m_VulkanSurface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Window Surface!");
	return m_VulkanSurface;
}

// Callback
void pom::Window::FrameBufferResizeCallback(GLFWwindow* window, int, int)
{
	Window* pWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
	pWindow->m_IsOutOfDate = true;
	glfwGetFramebufferSize(pWindow->m_pWindow, &pWindow->m_Size.x, &pWindow->m_Size.y);
}
