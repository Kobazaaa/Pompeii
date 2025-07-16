#ifndef SCENE_OBJECT_H
#define SCENE_OBJECT_H

// -- Standard Library --
#include <string>
#include <memory>
#include <algorithm>
#include <vector>

// -- Pompeii Includes --
#include "Transform.h"

// -- Forward Declarations --
namespace pompeii
{
	class Scene;
	class Component;
}
namespace pompeii
{
	// todo I have to do this to avoid a circular dependency, maybe fix this?
	struct ComponentDeleter { void operator()(const Component* ptr) const; };

	class SceneObject final
	{
	public:
		//--------------------------------------------------
		//    Constructors and Destructors
		//--------------------------------------------------
		explicit SceneObject(Scene& scene, std::string name = "SceneObject");
		~SceneObject() = default;

		SceneObject(const SceneObject& other) = delete;
		SceneObject(SceneObject&& other) = delete;
		SceneObject& operator=(const SceneObject& other) = delete;
		SceneObject& operator=(SceneObject&& other) = delete;


		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Start() const;
		void Update() const;
		void OnImGuiRender() const;

		//--------------------------------------------------
		//    Components
		//--------------------------------------------------
		template <typename ComponentType>
			requires std::derived_from<ComponentType, Component>
		[[nodiscard]] bool HasComponent() const
		{
			return std::ranges::any_of(m_vComponents, [](const auto& component) 
					{ return dynamic_cast<ComponentType>(component.get()); });
		}
		template <typename ComponentType>
			requires std::derived_from<ComponentType, Component>
		[[nodiscard]] ComponentType* GetComponent(int index = 0)
		{
			int componentCount = 0;
			for (const auto& component : m_vComponents)
			{
				if (auto castedComponent = dynamic_cast<ComponentType*>(component.get()))
				{
					if (componentCount == index) return castedComponent;
					++componentCount;
				}
			}
			return nullptr;
		}
		template <typename ComponentType, typename... Args>
			requires std::derived_from<ComponentType, Component>
		ComponentType* AddComponent(Args&&... args)
		{
			m_vComponents.push_back(std::make_unique<ComponentType>(*this, std::forward<Args>(args)...));
			return dynamic_cast<ComponentType*>(m_vComponents.back().get());
		}

		//--------------------------------------------------
		//    Data
		//--------------------------------------------------
		std::string name{ "SceneObject" };
		std::unique_ptr<Transform> transform = std::make_unique<Transform>(this);	
		Scene& GetScene() const;

		//--------------------------------------------------
		//    Flags
		//--------------------------------------------------
		bool IsFlaggedForDestruction() const;
		void Destroy();

		bool IsActive() const;
		void SetActive(bool active);

	private:
		// -- Component --
		void CleanupDeletedComponents();
		std::vector<std::unique_ptr<Component>> m_vComponents;

		// -- Scene --
		Scene* m_pScene{ nullptr };

		// -- Flags --
		bool m_IsActive{ true };
		bool m_DeletionFlag{ false };
	};
}

#endif // SCENE_OBJECT_H
