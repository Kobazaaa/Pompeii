// -- Pompeii Includes --
#include "LightComponent.h"
#include "ServiceLocator.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::LightComponent::LightComponent(SceneObject& parent, const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, LightType type)
	: LightComponent(parent,
		{
			.dirPos = dirPos,
			.color = col,
			.luxLumen = luxLumen,
			.type = type,
		})
{
	if (type == LightType::Point)
		GetTransform().SetPosition(dirPos);
}
pompeii::LightComponent::LightComponent(SceneObject& parent, LightCPU data)
	: Component(parent, "LightComponent")
	, lightData(std::move(data))
{
	lightData.CalculateLightMatrices(GetSceneObject().GetScene().GetAABB());
	ServiceLocator::Get<LightingSystem>().RegisterLight(*this);
}
pompeii::LightComponent::~LightComponent()
{
	ServiceLocator::Get<LightingSystem>().UnregisterLight(*this);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::LightComponent::Start()
{
	ServiceLocator::Get<RenderSystem>().GetRenderer()->UpdateLights();
}
void pompeii::LightComponent::OnInspectorDraw()
{
	ImGui::Text("Type"); ImGui::SameLine();
	if (ImGui::Combo("##Type", reinterpret_cast<int*>(&lightData.type), "Directional\0Point"))
	{
		lightData.CalculateLightMatrices(GetSceneObject().GetScene().GetAABB());
	}

	ImGui::Text("Intensity"); ImGui::SameLine();
	ImGui::InputFloat("##Intensity", &lightData.luxLumen);
	ImGui::Text("Color"); ImGui::SameLine();
	ImGui::ColorEdit3("##Color", &lightData.color.r);
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
LightHandle pompeii::LightComponent::GetLightHandle() const
{
	return m_LightHandle;
}
