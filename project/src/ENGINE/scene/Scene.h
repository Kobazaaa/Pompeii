#ifndef SCENE_H
#define SCENE_H

// -- Defines --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS

// -- Standard Library --
#include <memory>

// -- Pom Includes --
#include "SceneObject.h"
#include "Shapes.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Base Scene	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Scene final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit Scene(std::string sceneName);
		~Scene() = default;

		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = delete;
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = delete;

		//--------------------------------------------------
		//    Adding & Removing SceneObject
		//--------------------------------------------------
		[[nodiscard]] SceneObject& AddEmpty(const std::string& sceneName = "SceneObject");

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() const;
		void Update();

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		std::string name{ "EmptyScene" };
		std::vector<SceneObject*> GetObjectsByName(const std::string& objectName) const;
		std::vector<SceneObject*> GetAllObjects() const;
		const AABB& GetAABB() const;
		void GrowAABB(const AABB& aabb);

	private:
		void CleanupDeletedObjects();
		void AddPendingObjects();

		std::vector<std::unique_ptr<SceneObject>> m_vObjects{};
		std::vector<std::unique_ptr<SceneObject>> m_vPendingObjects{};
		AABB m_AABB;
	};
}
#endif // SCENE_H