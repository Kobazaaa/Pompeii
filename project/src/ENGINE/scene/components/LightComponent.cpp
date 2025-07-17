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
{}
pompeii::LightComponent::LightComponent(SceneObject& parent, LightCPU data)
	: Component(parent)
	, lightData(std::move(data))
{
	lightData.CalculateLightMatrices(GetSceneObject().GetScene().GetAABB());
	ServiceLocator::Get<RenderSystem>().RegisterLight(*this);
}
pompeii::LightComponent::~LightComponent()
{
	ServiceLocator::Get<RenderSystem>().UnregisterLight(*this);
}


//--------------------------------------------------
//    Loop
//--------------------------------------------------
void pompeii::LightComponent::Start()
{
	ServiceLocator::Get<RenderSystem>().GetRenderer()->UpdateLights();
}
void pompeii::LightComponent::OnImGuiRender()
{
}


//--------------------------------------------------
//    Accessors & Mutators
//--------------------------------------------------
LightHandle pompeii::LightComponent::GetLightHandle() const
{
	return m_LightHandle;
}
