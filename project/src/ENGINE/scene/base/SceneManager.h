#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

// -- Standard Library --
#include <vector>
#include <string>
#include <memory>

// -- Pompeii Includes --
#include "Scene.h"

namespace pompeii
{
	class SceneManager final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit SceneManager() = default;
		Scene& CreateScene(const std::string& name);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		Scene* GetScene(uint32_t index) const;
		Scene* GetScene(const std::string& name) const;
		Scene& GetActiveScene() const;
		void SetActiveScene(Scene& scene);
		void SetActiveScene(const std::string& name);

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() const;
		void Update() const;

	private:
		std::vector<std::unique_ptr<Scene>> m_vScenes;
		Scene* m_pActiveScene{};
	};
}

#endif // SCENE_MANAGER_H
