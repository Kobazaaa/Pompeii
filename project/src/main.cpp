#include <iostream>
#include <stdexcept>
#include <cstdlib>

#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include "Renderer.h"
#include "DeletionQueue.h"
#include "Window.h"


using namespace pom;

int main()
{
	// -- Create Application Level Deletion Queue --
	DeletionQueue m_ApplicationDQ{};


	// -- Create Window --
	Window* pWindow = new Window();
	{
		pWindow->Initialize(800, 600, "Vulkan Refactored");
		m_ApplicationDQ.Push([&] { pWindow->Destroy(); delete pWindow; });
	}


	// -- Create Renderer --
	Renderer* pRenderer = new Renderer();
	{
		pRenderer->Initialize(pWindow);
		m_ApplicationDQ.Push([&] { pRenderer->Destroy(); delete pRenderer; });
	}


	// -- Try to run lol --
	try
	{
		// -- Main Loop --
		while (!glfwWindowShouldClose(pWindow->GetWindow()))
		{
			glfwPollEvents();
			pRenderer->Update();
			pRenderer->Render();
		}
	}
	// -- Catch Failures --
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";

		m_ApplicationDQ.Flush();
		return EXIT_FAILURE;
	}


	// -- Cleanup --
	m_ApplicationDQ.Flush();
	return EXIT_SUCCESS;
}
