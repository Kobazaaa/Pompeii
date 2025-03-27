#include "Window.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pom::Window::Window(int width, int height, const char* title)
	: m_Size(width, height)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwSetWindowUserPointer(m_pWindow, this);
	glfwSetFramebufferSizeCallback(m_pWindow, FrameBufferResizeCallback);
}
pom::Window::~Window()
{
	glfwDestroyWindow(m_pWindow);
	glfwTerminate();
}



//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
GLFWwindow* pom::Window::GetWindow()	const	{ return m_pWindow; }
glm::ivec2 pom::Window::GetSize()		const	{ return m_Size; }
bool pom::Window::IsOutdated()			const	{ return m_IsOutOfDate; }
void pom::Window::ResetOutdated()				{ m_IsOutOfDate = false; }

// Callback
void pom::Window::FrameBufferResizeCallback(GLFWwindow* window, int width, int height)
{
	Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	pWindow->m_IsOutOfDate = true;
	glfwGetFramebufferSize(pWindow->m_pWindow, &pWindow->m_Size.x, &pWindow->m_Size.y);
}
