// -- Pompeii Includes --
#include "Scene.h"

// -- Standard Library --
#include <numeric>
#include <ranges>

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Base Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::Scene::AllocateGPU(const Context& context, bool keepHostData)
{
	for (auto& model : m_vModels)
		model.AllocateResources(context, keepHostData);

	if (!m_EnvMapPath.empty())
		m_EnvironmentMap
			.CreateSampler(context)
			.CreateSkyboxCube(context, m_EnvMapPath)
			.CreateDiffIrradianceMap(context)
			.CreateSpecIrradianceMap(context);
}
void pom::Scene::Destroy(const Context& context)
{
	for (auto& model : std::ranges::reverse_view(m_vModels))
		model.Destroy(context);
	m_EnvironmentMap.Destroy(context);
}

// -- Model --
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

// -- Light --
std::vector<pom::Light>& pom::Scene::GetLights()		{ return m_vLights; }
std::vector<pom::GPULight> pom::Scene::GetLightsGPU()	{ return m_vGPULights; }
uint32_t pom::Scene::GetLightsCount() const				{ return static_cast<uint32_t>(m_vLights.size()); }
pom::Light& pom::Scene::AddLight(const Light& light)
{
	m_vLights.push_back(light);
	m_vGPULights.push_back(light.GetGPULight());
	return m_vLights.back();
}
void pom::Scene::PopLight()
{
	if (m_vLights.empty() || m_vGPULights.empty())
		return;
	m_vLights.pop_back();
	m_vGPULights.pop_back();
}

// -- Environment Map --
const pom::EnvironmentMap& pom::Scene::GetEnvironmentMap() const { return m_EnvironmentMap; }
void pom::Scene::SetEnvironmentMap(const std::string& path) { m_EnvMapPath = path; }


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Sponza Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::SponzaScene::Initialize()
{
	AddModel("models/Sponza.gltf");
	SetEnvironmentMap("textures/circus_arena_4k.hdr");

	//AddLight(Light
	//	{
	//	/* direction */	{ 0.f, -1.f, 0.f },
	//	/* color */		{ 1.f, 1.f, 1.f },
	//	/* lux */		1.f, Light::Type::Directional //todo sunlight at noon is about 100.000 lux
	//	});
	//AddLight(Light
	//	{
	//	/* direction */	{ -0.577f, -0.577f, -0.577f },
	//	/* color */		{ 1.f, 0.f, 1.f },
	//	/* lux */		1.f, Light::Type::Directional
	//	});
	AddLight(Light
		{
		/* position */	{ 3.f, 0.5f, 0.f },
		/* color */		{ 0.f, 1.f, 0.f },
		/* lumen */		15.f, Light::Type::Point
		});
	AddLight(Light
		{
		/* position */	{ 7.f, 0.5f, 0.f },
		/* color */		{ 1.f, 1.f, 0.f},
		/* lumen */		100.f, Light::Type::Point
		});
}


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  FlightHelmet Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::FlightHelmetScene::Initialize()
{
	AddModel("models/FlightHelmet.gltf");

	AddLight(Light
		{
			/* direction */	{ 0.577f, -0.577f, 0.577f },
			/* color */		{ 1.f, 1.f, 1.f },
			/* lux */		1.f, Light::Type::Directional
		});
}

