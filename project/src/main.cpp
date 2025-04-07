#include <iostream>
#include <stdexcept>
#include <cstdlib>

#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_LEFT_HANDED

#include "Renderer.h"
#include "DeletionQueue.h"
#include "Window.h"
#include "Camera.h"
#include "Timer.h"


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


	// -- Create Camera --
	CameraSettings settings
	{
		.fov = 45.f,
		.aspectRatio = pWindow->GetAspectRatio(),
		.nearPlane = 0.1f,
		.farPlane = 1000.f
	};
	Camera* pCamera = new Camera(settings, pWindow);
	m_ApplicationDQ.Push([&] { delete pCamera; });


	// -- Create Renderer --
	Renderer* pRenderer = new Renderer();
	{
		pRenderer->Initialize(pCamera, pWindow);
		m_ApplicationDQ.Push([&] { pRenderer->Destroy(); delete pRenderer; });
	}


	// -- Try to run lol --
	try
	{
		// -- Main Loop --
		Timer::Start();
		while (!glfwWindowShouldClose(pWindow->GetWindow()))
		{
			Timer::Update();
			glfwPollEvents();
			pCamera->Update();
			pRenderer->Update();
			pRenderer->Render();

			std::this_thread::sleep_for(Timer::SleepDurationNanoSeconds());
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
