// -- Standard Library --
#include <string>

// -- Pompeii Includes --
#include "SceneObject.h"
#include "Renderer.h"

//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
pompeii::SceneObject::SceneObject(std::string name)
	: name(std::move(name))
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
		child->GetOwner()->Destroy();
}

bool pompeii::SceneObject::IsActive() const
{
	return m_IsActive;
}
void pompeii::SceneObject::SetActive(bool active)
{
	m_IsActive = active;
	for (auto& child : transform->GetAllChildren())
		child->GetOwner()->SetActive(active);
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

