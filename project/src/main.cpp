// -- Standard Library --
#include <iostream>
#include <cstdlib>

// -- Defines --
#define VMA_IMPLEMENTATION
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Pompeii Includes
#include "Renderer.h"
#include "Window.h"
#include "Camera.h"
#include "ConsoleTextSettings.h"
#include "Timer.h"
#include "ServiceLocator.h"

// -- Using Pompeii namespace --
using namespace pompeii;

int main()
{
	// -- Try to run lol --
	try
	{
		// -- Create Window --
		Window* pWindow = new Window("V - Pompeii", false, 800, 600);

		// -- Create Camera --
		CameraSettings settings
		{
			.fov = 45.f,
			.aspectRatio = pWindow->GetAspectRatio(),
			.nearPlane = 0.001f,
			.farPlane = 1000.f
		};
		ExposureSettings sunny16
		{
			.aperture = 16.f,
			.shutterSpeed = 1.f / 100.f,
			.iso = 100.f,
		};
		ExposureSettings indoor
		{
			.aperture = 1.4f,
			.shutterSpeed = 1.f / 60.f,
			.iso = 1600.f,
		};

		// -- Register Services --
		ServiceLocator::RegisterSceneManager(std::make_unique<SceneManager>());

		// -- Create Test Scene --
		auto& scene = ServiceLocator::GetSceneManager().CreateScene("TestScene");
		ServiceLocator::GetSceneManager().SetActiveScene(scene);

		// cam
		auto& camera = scene.AddEmpty("Camera");
		camera.AddComponent<Camera>(settings, sunny16, pWindow, true);

		// model
		auto& model = scene.AddEmpty("Model");
		model.AddComponent<Model>("models/Sponza.gltf");

		// light
		auto& light = scene.AddEmpty("Light");
		light.AddComponent<Light>(
			/* direction */	glm::vec3{ 0.f, -1.f, 0.f },
			/* color */		glm::vec3{ 1.f, 1.f, 1.f },
			/* lux */			100'000.f, Light::Type::Directional
		);

		ServiceLocator::RegisterRenderer(std::make_unique<Renderer>(pWindow));

		// -- Main Loop --
		ServiceLocator::GetSceneManager().Start();
		Timer::Start();
		bool wasPressed = false;
		bool isPressed = false;
		float printTime = 1.f;

		while (!glfwWindowShouldClose(pWindow->GetHandle()))
		{
			// -- Timer Update --
			Timer::Update();

			// -- Input Update --
			isPressed = glfwGetKey(pWindow->GetHandle(), GLFW_KEY_F11) == GLFW_PRESS;
			if (isPressed && !wasPressed)
				pWindow->ToggleFullScreen();
			wasPressed = isPressed;
			if (pWindow->IsFullScreen() && glfwGetKey(pWindow->GetHandle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
				pWindow->ToggleFullScreen();

			glfwPollEvents();

			// -- Update --
			ServiceLocator::GetSceneManager().Update();
			ServiceLocator::GetRenderer().Render();

			// -- Print --
			printTime -= Timer::GetDeltaSeconds();
			if (printTime <= 0)
			{
				printTime = 1.0f;
				std::cout << DARK_YELLOW_TXT << "dFPS: " << 1.0 / Timer::GetDeltaSeconds() << "\t(" << Timer::GetDeltaSeconds() * 1000 << "ms)\n" << RESET_TXT;
			}

			//std::this_thread::sleep_for(Timer::SleepDurationNanoSeconds());
		}

		// -- Cleanup --
		ServiceLocator::DeregisterSceneManager();
		ServiceLocator::DeregisterRenderer();
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
