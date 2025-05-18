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

std::vector<pom::DirectionalLight>& pom::Scene::GetDirectionalLights()
{
	return m_vLights;
}
std::vector<pom::Scene::GPULight> pom::Scene::GetLightsGPU()
{
	std::vector<GPULight> gpuLights;
	gpuLights.reserve(m_vLights.size());
	std::ranges::transform(m_vLights,
	                       std::back_inserter(gpuLights),
	                       [](const DirectionalLight& light) {
		                       return GPULight{ .dirPosType = glm::vec4(light.GetDirection(), 0.f), .color = light.GetColor(), .intensity = light.GetIntensity() };
	                       }
	);
	return gpuLights;
}
uint32_t pom::Scene::GetLightsCount() const
{
	return static_cast<uint32_t>(m_vLights.size());
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Sponza Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::SponzaScene::Initialize()
{
	AddModel("models/Sponza.gltf");

	m_vLights.emplace_back(DirectionalLight{
		/* direction */	{ 0.577f, -0.577f, 0.577f },
		/* color */		{ 1.f, 1.f, 1.f },
		/* intensity */	1.f });
	m_vLights.emplace_back(DirectionalLight{
		/* direction */	{ -0.577f, -0.577f, -0.577f },
		/* color */		{ 1.f, 1.f, 1.f },
		/* intensity */	1.f
});
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FlightHelmet Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::FlightHelmetScene::Initialize()
{
	AddModel("models/FlightHelmet.gltf");
}

