#ifndef SERVICE_LOCATOR_H
#define SERVICE_LOCATOR_H

// -- Pompeii Includes --
#include "SceneManager.h"
#include "Renderer.h"

namespace pompeii
{
	class ServiceLocator final
	{
	public:
		//--------------------------------------------------
		//    Constructors and Destructors
		//--------------------------------------------------
		ServiceLocator() = delete;
		~ServiceLocator() = delete;

		ServiceLocator(const ServiceLocator& other) = delete;
		ServiceLocator(ServiceLocator&& other) = delete;
		ServiceLocator& operator=(const ServiceLocator& other) = delete;
		ServiceLocator& operator=(ServiceLocator&& other) = delete;

		//--------------------------------------------------
		//    Services
		//--------------------------------------------------
		// -- SceneManager --
		static SceneManager& GetSceneManager() { return *m_pSceneManagerService; }
		static void RegisterSceneManager(std::unique_ptr<SceneManager>&& service) { m_pSceneManagerService = std::move(service); }
		// -- Renderer --
		static Renderer& GetRenderer() { return *m_pRendererService; }
		static void RegisterRenderer(std::unique_ptr<Renderer>&& service) { m_pRendererService = std::move(service); }

	private:
		inline static std::unique_ptr<SceneManager> m_pSceneManagerService {};
		inline static std::unique_ptr<Renderer> m_pRendererService {};
	};
}

#endif // SERVICE_LOCATOR_H
