#ifndef COMPONENT_H
#define COMPONENT_H

// -- Pompeii Includes --
#include "SceneObject.h"

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
		explicit Component(SceneObject& sceneObj) : m_pSceneObj{ &sceneObj } {}
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
		void Destroy()											{ m_DeletionFlag = true; }
		[[nodiscard]] bool IsFlaggedForDestruction()	const	{ return m_DeletionFlag; }
		[[nodiscard]] SceneObject& GetSceneObject()		const	{ return *m_pSceneObj; }
		[[nodiscard]] Transform& GetTransform()			const	{ return *m_pSceneObj->transform; }
		bool isActive = true;

	private:
		SceneObject* m_pSceneObj	{ nullptr };
		bool m_DeletionFlag			{ false };
	};
}

#endif // COMPONENT_H
