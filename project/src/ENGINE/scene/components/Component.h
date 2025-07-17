#ifndef COMPONENT_H
#define COMPONENT_H

// -- Pompeii Includes --
#include "Transform.h"

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
		explicit Component(SceneObject& sceneObj);
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
		virtual void OnImGuiRender() {}

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		void Destroy();
		[[nodiscard]] bool IsFlaggedForDestruction()	const;
		[[nodiscard]] SceneObject& GetSceneObject()		const;
		[[nodiscard]] Transform& GetTransform()			const;
		bool isActive = true;

	private:
		SceneObject* m_pSceneObj	{ nullptr };
		bool m_DeletionFlag			{ false };
	};
}

#endif // COMPONENT_H
