// -- Pompeii Includes --
#include "RenderSystem.h"
#include "Model.h"

// -- Standard Library --
#include <numeric>

#include "Scene.h"

//--------------------------------------------------
//    Models
//--------------------------------------------------
void pompeii::RenderSystem::RegisterModel(Model& model)
{
	if (std::ranges::find(m_vRegisteredModels, &model) != m_vRegisteredModels.end())
		return;
	m_vRegisteredModels.emplace_back(&model);
}
void pompeii::RenderSystem::UnregisterModel(const Model& model)
{
	std::erase_if(m_vRegisteredModels, [&](const Model* pModel) { return pModel == &model; });
}

const std::vector<pompeii::Model*>& pompeii::RenderSystem::GetVisibleModels() const
{
	return m_vVisibleModels;
}
const std::vector<pompeii::Model*>& pompeii::RenderSystem::GetAllModels() const
{
	return m_vRegisteredModels;
}


//--------------------------------------------------
//    Textures
//--------------------------------------------------
uint32_t pompeii::RenderSystem::GetTextureCount() const
{
	return std::accumulate(m_vRegisteredModels.begin(), m_vRegisteredModels.end(), 0u, [](uint32_t res, const Model* m2)
		{
			return res + static_cast<uint32_t>(m2->images.size());
		});
}

//--------------------------------------------------
//    Camera
//--------------------------------------------------
pompeii::Camera* pompeii::RenderSystem::GetMainCamera() const
{
	return m_pMainCamera;
}
void pompeii::RenderSystem::SetMainCamera(Camera& camera)
{
	m_pMainCamera = &camera;
}

//--------------------------------------------------
//    Interface
//--------------------------------------------------
void pompeii::RenderSystem::BeginFrame()
{
	FrustumCull();
}
void pompeii::RenderSystem::EndFrame()
{
	m_vVisibleModels.clear();
}


//--------------------------------------------------
//    Helpers
//--------------------------------------------------

void pompeii::RenderSystem::FrustumCull()
{
	for (Model* pModel : m_vRegisteredModels)
	{
		m_vVisibleModels.push_back(pModel);
	}
}
