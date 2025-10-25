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
#include "ServiceLocator.h"
#include "Timer.h"
#include "Editor.h"
#include "AssetManager.h"
#include "LightComponent.h"
#include "MeshRenderer.h"
#include "GPUCamera.h"

// -- Using Pompeii namespace --
using namespace pompeii;

static void CreateDefaultScene(Window* pWindow)
{
	auto& scene = ServiceLocator::Get<SceneManager>().CreateScene("DefaultScene");
	ServiceLocator::Get<SceneManager>().SetActiveScene(scene);

	// cam
	auto& camera = scene.AddEmpty("Camera");
	camera.AddComponent<Camera>(CameraSettings{ .fov = 45.f, .aspectRatio = pWindow->GetAspectRatio(), .nearPlane = 0.001f, .farPlane = 1000.f }, pWindow, true);

	// model
	auto& model = scene.AddEmpty("Model");
	auto filter = model.AddComponent<MeshFilter>();
	Mesh* pMesh = ServiceLocator::Get<AssetManager>().LoadMesh("models/ABeautifulGame.gltf");
	filter->pMesh = pMesh;
	model.AddComponent<MeshRenderer>(*filter);

	// light
	auto& light1 = scene.AddEmpty("SunLight");
	light1.AddComponent<LightComponent>(
			/* direction */	glm::vec3{ 0.577f, -0.577f, 0.577f },
			/* color */		glm::vec3{ 1.f, 1.f, 1.f },
			/* lux */			20.f, LightType::Directional
	);

	auto& light2 = scene.AddEmpty("GreenLight");
	light2.AddComponent<LightComponent>(
			/* position */		glm::vec3{ 3.f, 0.5f, 0.f },
			/* color */		glm::vec3{ 0.f, 1.f, 0.f },
			/* lumen */		1000.f, LightType::Point
	);

	auto& light3 = scene.AddEmpty("YellowLight");
	light3.AddComponent<LightComponent>(
			/* position */		glm::vec3{ 7.f, 0.5f, 0.f },
			/* color */		glm::vec3{ 1.f, 1.f, 0.f},
			/* lumen */		1200.f, LightType::Point
	);
}

int main()
{
	// -- Try to run lol --
	try
	{
		// -- Create Window --
		Window* pWindow = new Window("V - Pompeii", false, 1920, 1080);

		// -- Register Services --
		auto renderer = std::make_shared<Renderer>(pWindow);
		ServiceLocator::Register(std::make_unique<SceneManager>());
		ServiceLocator::Register(std::make_unique<RenderSystem>());
		ServiceLocator::Register(std::make_unique<LightingSystem>());
		ServiceLocator::Register(std::make_unique<Editor>());
		ServiceLocator::Register(std::make_unique<AssetManager>());

		ServiceLocator::Get<LightingSystem>().SetRenderer(renderer);
		ServiceLocator::Get<RenderSystem>().SetRenderer(renderer);
		ServiceLocator::Get<AssetManager>().SetRenderer(renderer);
		renderer->InsertUI([]
			{
				ServiceLocator::Get<Editor>().Draw();
			});


		// -- Create Default Scene --
		CreateDefaultScene(pWindow);

		// -- Main Loop --
		ServiceLocator::Get<SceneManager>().Start();
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



			// --- Begin Frame Phase ---
			ServiceLocator::Get<LightingSystem>().BeginFrame();
			ServiceLocator::Get<RenderSystem>().BeginFrame();

			// -- Update Phase --
			ServiceLocator::Get<SceneManager>().Update();
			ServiceLocator::Get<LightingSystem>().Update();
			ServiceLocator::Get<RenderSystem>().Update();

			// -- Render Phase --
			renderer->Render();

			// -- End Frame Phase --
			ServiceLocator::Get<LightingSystem>().EndFrame();
			ServiceLocator::Get<RenderSystem>().EndFrame();
			 


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
		ServiceLocator::Get<RenderSystem>().GetRenderer()->GetContext().device.WaitIdle();
		ServiceLocator::Deregister<SceneManager>();
		ServiceLocator::Get<AssetManager>().UnloadAll();
		ServiceLocator::Deregister<AssetManager>();
		ServiceLocator::Deregister<LightingSystem>();
		ServiceLocator::Deregister<RenderSystem>();
		renderer.reset();
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
