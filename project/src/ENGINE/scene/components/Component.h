#ifndef COMPONENT_H
#define COMPONENT_H

// -- Standard Library --
#include <string>

// -- Pompeii Includes --
#include "Transform.h"

// -- ImGui --
#include "imgui.h"

// -- Forward Declarations --
namespace pompeii
{
	class SceneObject;
}
namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Component	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Component
	{
	public:
		//--------------------------------------------------
		//    Constructor and Destructor
		//--------------------------------------------------
		explicit Component(SceneObject& sceneObj, const std::string& name);
		virtual ~Component() = default;
		Component(const Component& other) = delete;
		Component(Component&& other) = delete;
		Component& operator=(const Component& other) = delete;
		Component& operator=(Component&& other) = delete;


		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		virtual void Start() {}
		virtual void Update() {}
		virtual void OnInspectorDraw() {}

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		void Destroy();
		[[nodiscard]] bool IsFlaggedForDestruction()	const;
		[[nodiscard]] SceneObject& GetSceneObject()		const;
		[[nodiscard]] Transform& GetTransform()			const;
		bool isActive = true;
		std::string name { "Unknown" };

	private:
		SceneObject* m_pSceneObj	{ nullptr };
		bool m_DeletionFlag			{ false };
	};
}

#endif // COMPONENT_H
