// -- Standard Library --
#include <string>

// -- Pompeii Includes --
#include "SceneObject.h"

// -- ImGui --
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

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
void pompeii::SceneObject::OnInspectorDraw()
{
	// -- Info --
	ImGui::Text("Name:");
	ImGui::SameLine();
	ImGui::InputText("##Name", &name);
	ImGui::SameLine();
	if (ImGui::Checkbox("##Active", &m_IsActive))
		SetActive(m_IsActive);
	ImGui::Separator();

	// -- Transform --
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		glm::vec3 pos = transform->GetPosition();
		glm::vec3 rot = transform->GetEulerAngles();
		glm::vec3 scale = transform->GetScale();
		bool changed = false;

		ImGui::Text("Position"); ImGui::SameLine();
		changed |= ImGui::DragFloat3("##Position", &pos.x, 0.01f);

		ImGui::Text("Rotation"); ImGui::SameLine();
		changed |= ImGui::DragFloat3("##Rotation", &rot.x, 0.01f);

		ImGui::Text("Scale"); ImGui::SameLine();
		changed |= ImGui::DragFloat3("##Scale", &scale.x, 0.01f);

		if (changed)
		{
			transform->SetPosition(pos);
			transform->SetEulerAngles(rot);
			transform->SetScale(scale);
		}

		ImGui::Separator();
	}

	// -- Other Components --
	for (const auto& component : m_vComponents)
	{
		if (!component) continue;
		if (ImGui::CollapsingHeader(component->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			component->OnInspectorDraw();
		ImGui::Separator();
	}
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

