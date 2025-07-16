// -- Standard Library --
#include <string>

// -- Pompeii Includes --
#include "SceneObject.h"
#include "Component.h"

void pompeii::ComponentDeleter::operator()(const Component* ptr) const { delete ptr; }

//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
pompeii::SceneObject::SceneObject(Scene& scene, std::string name)
	: name(std::move(name))
	, m_pScene(&scene)
{}

//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::SceneObject::Start() const
{
	for (const auto& component : m_vComponents)
		if (component->isActive)
			component->Start();
}
void pompeii::SceneObject::Update() const
{
	for (const auto& component : m_vComponents)
		if (component->isActive)
			component->Update();
}
void pompeii::SceneObject::OnImGuiRender() const
{
	for (const auto& component : m_vComponents)
		if (component->isActive)
			component->OnImGuiRender();
}

//--------------------------------------------------
//    Data
//--------------------------------------------------
pompeii::Scene& pompeii::SceneObject::GetScene() const
{
	return *m_pScene;
}

//--------------------------------------------------
//    Flags
//--------------------------------------------------
bool pompeii::SceneObject::IsFlaggedForDestruction() const
{
	return m_DeletionFlag;
}
void pompeii::SceneObject::Destroy()
{
	m_DeletionFlag = true;
	for (const auto& child : transform->GetAllChildren())
		child->GetSceneObject()->Destroy();
}

bool pompeii::SceneObject::IsActive() const
{
	return m_IsActive;
}
void pompeii::SceneObject::SetActive(bool active)
{
	m_IsActive = active;
	for (auto& child : transform->GetAllChildren())
		child->GetSceneObject()->SetActive(active);
}


//--------------------------------------------------
//    Deletion
//--------------------------------------------------

// -- Component --
void pompeii::SceneObject::CleanupDeletedComponents()
{
	std::erase_if(m_vComponents,
	              [](const std::unique_ptr<Component>& component)
	              {
		              return component->IsFlaggedForDestruction();
	              });
}

