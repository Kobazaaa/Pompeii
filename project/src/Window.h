#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

namespace pom
{
	class Window final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Window(int width, int height, const char* title);
		~Window();

		Window(const Window& other) = delete;
		Window(Window&& other) noexcept = delete;
		Window& operator=(const Window& other) = delete;
		Window& operator=(Window&& other) noexcept = delete;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		GLFWwindow* GetWindow()		const;
		glm::ivec2	GetSize()		const;

		bool		IsOutdated()	const;
		void		ResetOutdated();

	private:
		// Callback for when the window gets resized
		static void FrameBufferResizeCallback(GLFWwindow* window, int width, int height);

		GLFWwindow* m_pWindow		{ nullptr };
		glm::ivec2	m_Size			{ };
		bool		m_IsOutOfDate	{ false };
	};
}
#endif // WINDOW_H
