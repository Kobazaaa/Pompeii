// -- Standard Library --
#include <iostream>
#include <stdexcept>
#include <cstdlib>

// -- Defines --
#define VMA_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RADIANS

// -- Pompeii Includes
#include "Renderer.h"
#include "Window.h"
#include "Camera.h"
#include "ConsoleTextSettings.h"
#include "Timer.h"

// -- Using Pompeii namespace --
using namespace pom;

int main()
{
	// -- Try to run lol --
	try
	{
		// -- Create Window --
		Window* pWindow = new Window("V - Pompeii - Kobe Dereyne 2GD10", false, 800, 600);

		// -- Create Camera --
		CameraSettings settings
		{
			.fov = 45.f,
			.aspectRatio = pWindow->GetAspectRatio(),
			.nearPlane = 0.001f,
			.farPlane = 10000.f
		};
		Camera* pCamera = new Camera(settings, pWindow);

		// -- Create Renderer --
		Renderer* pRenderer = new Renderer(pCamera, pWindow);

		// -- Main Loop --
		Timer::Start();
		bool wasPressed = false;
		bool isPressed = false;
		float printTime = 1.f;

		while (!glfwWindowShouldClose(pWindow->GetHandle()))
		{
			Timer::Update();

			isPressed = glfwGetKey(pWindow->GetHandle(), GLFW_KEY_F11) == GLFW_PRESS;
			if (isPressed && !wasPressed)
				pWindow->ToggleFullScreen();
			wasPressed = isPressed;
			if (pWindow->IsFullScreen() && glfwGetKey(pWindow->GetHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				pWindow->ToggleFullScreen();

			glfwPollEvents();
			pCamera->Update();
			pRenderer->Update();
			pRenderer->Render();

			printTime -= Timer::GetDeltaSeconds();
			if (printTime <= 0)
			{
				printTime = 1.0f;
				std::cout << DARK_YELLOW_TXT << "dFPS: " << 1.0 / Timer::GetDeltaSeconds() << "\t(" << Timer::GetDeltaSeconds() * 1000 << "ms)\n" << RESET_TXT;
			}

			//std::this_thread::sleep_for(Timer::SleepDurationNanoSeconds());
		}

		// -- Cleanup --
		delete pRenderer;
		delete pCamera;
		delete pWindow;
	}
	// -- Catch Failures --
	catch (const std::exception& e)
	{
		std::cerr << ERROR_TXT << e.what() << RESET_TXT << "\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
