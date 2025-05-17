// -- Pompeii Includes --
#include "Scene.h"

// -- Standard Library --
#include <numeric>
#include <ranges>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Base Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::Scene::AllocateGPU(const Context& context, CommandPool& cmdPool, bool keepHostData)
{
	for (auto& model : m_vModels)
		model.AllocateResources(context, cmdPool, keepHostData);
}
void pom::Scene::Destroy(const Context& context)
{
	for (auto& model : std::ranges::reverse_view(m_vModels))
		model.Destroy(context);
}

const std::vector<pom::Model>& pom::Scene::GetModels() const
{
	return m_vModels;
}
pom::Model& pom::Scene::AddModel(const std::string& path)
{
	m_vModels.emplace_back();
	m_vModels.back().LoadModel(path);
	return m_vModels.back();
}
uint32_t pom::Scene::GetImageCount() const
{
	return std::accumulate(m_vModels.begin(), m_vModels.end(), 0u, [](uint32_t res, const Model& m2)
		{
			return res + static_cast<uint32_t>(m2.images.size());
		});
}

std::vector<pom::DirectionalLight>& pom::Scene::GetLights()
{
	return m_vLights;
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Sponza Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::SponzaScene::Initialize()
{
	AddModel("models/Sponza.gltf");

	m_vLights.emplace_back(DirectionalLight
		{ /* direction */	{ 0.577f, -0.577f, 0.577f },
		/* color */		{ 1.f, 1.f, 1.f },
		/* intensity */	1.f,
		/* size */		{ 5000.f, 5000.f },
		/* near-far */	{ 0.1f, 11000.f },
		/* distance */	10000.f });
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FlightHelmet Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::FlightHelmetScene::Initialize()
{
	AddModel("models/FlightHelmet.gltf");
}

