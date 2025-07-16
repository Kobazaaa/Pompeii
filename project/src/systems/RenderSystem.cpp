// -- Pompeii Includes --
#include "RenderSystem.h"
#include "ModelRenderer.h"

// -- Standard Library --
#include <numeric>

//--------------------------------------------------
//    Models
//--------------------------------------------------
void pompeii::RenderSystem::RegisterModel(ModelRenderer& model)
{
	if (std::ranges::find(m_vRegisteredModels, &model) != m_vRegisteredModels.end())
		return;
	m_vRegisteredModels.emplace_back(&model);
}
void pompeii::RenderSystem::UnregisterModel(const ModelRenderer& model)
{
	std::erase_if(m_vRegisteredModels, [&](const ModelRenderer* pModel) { return pModel == &model; });
}

const std::vector<pompeii::ModelRenderer*>& pompeii::RenderSystem::GetVisibleModels() const
{
	return m_vVisibleModels;
}
const std::vector<pompeii::ModelRenderer*>& pompeii::RenderSystem::GetAllModels() const
{
	return m_vRegisteredModels;
}


//--------------------------------------------------
//    Textures
//--------------------------------------------------
uint32_t pompeii::RenderSystem::GetTextureCount() const
{
	return std::accumulate(m_vRegisteredModels.begin(), m_vRegisteredModels.end(), 0u, [](uint32_t res, const ModelRenderer* m2)
		{
			return res + static_cast<uint32_t>(m2->GetModel()->images.size());
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
void pompeii::RenderSystem::Update()
{
}

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
	for (ModelRenderer* pModel : m_vRegisteredModels)
	{
		m_vVisibleModels.push_back(pModel);
	}
}
