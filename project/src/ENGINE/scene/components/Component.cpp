// -- Pompeii Includes --
#include "Component.h"
#include "SceneObject.h"
pompeii::Component::Component(SceneObject& sceneObj, const std::string& name) : name{ name }, m_pSceneObj{ &sceneObj } {}
void pompeii::Component::Destroy()									{ m_DeletionFlag = true; }
bool pompeii::Component::IsFlaggedForDestruction()			const	{ return m_DeletionFlag; }
pompeii::SceneObject& pompeii::Component::GetSceneObject()	const	{ return *m_pSceneObj; }
pompeii::Transform& pompeii::Component::GetTransform()		const	{ return *m_pSceneObj->transform; }
