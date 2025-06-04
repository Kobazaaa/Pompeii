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
			.CreateSpecIrradianceMap(context)
			.CreateBRDFLut(context);
	GenerateDepthMaps(context);
}
void pom::Scene::Destroy(const Context& context)
{
	for (auto& l : std::ranges::reverse_view(m_vLights))
		l.DestroyDepthMap(context);
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
	m_AABB.GrowToInclude(m_vModels.back().aabb);
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
std::vector<pom::GPULight> pom::Scene::GetLightsGPU()
{
	std::vector<pom::GPULight> res{};
	res.resize(m_vLights.size());
	for (uint32_t i{}; i < m_vLights.size(); ++i)
	{
		const auto& l = m_vLights[i];
		GPULight gpuL{};
		Light::Type type = l.GetType();
		gpuL.dirPosType = { type == Light::Type::Point ? l.dirPos : normalize(l.dirPos), static_cast<int>(l.GetType())};
		gpuL.intensity = l.luxLumen;
		gpuL.color = l.color;
		gpuL.shadowMatrixOffset = i == 0 ? 0 : res[i - 1].shadowMatrixCount + res[i - 1].shadowMatrixOffset;
		gpuL.shadowMatrixCount = type == Light::Type::Point ? 6 : 1;
		res.push_back(std::move(gpuL));
	}
	return res;
}
std::vector<glm::mat4> pom::Scene::GetLightMatrices()
{
	std::vector<glm::mat4> res{};
	for (auto& l : m_vLights)
	{
		const auto& proj = l.projMatrix;
		for (const auto& v : l.viewMatrices)
			res.push_back(proj * v);
	}
	return res;
}
uint32_t pom::Scene::GetLightsCount() const				{ return static_cast<uint32_t>(m_vLights.size()); }
pom::Light& pom::Scene::AddLight(Light&& light)
{
	m_vLights.push_back(std::move(light));
	return m_vLights.back();
}
void pom::Scene::PopLight()
{
	if (m_vLights.empty())
		return;
	m_vLights.pop_back();
}

void pom::Scene::CalculateLightMatrices()
{
	for (auto& light : m_vLights)
	{
		// -- Directional --
		if (light.GetType() == Light::Type::Directional)
		{
			const glm::vec3 center = (m_AABB.min + m_AABB.max) * 0.5f;
			const glm::vec3 lightDir = light.dirPos;
			const std::vector<glm::vec3> corners = {
				{m_AABB.min.x, m_AABB.min.y, m_AABB.min.z},
				{m_AABB.max.x, m_AABB.min.y, m_AABB.min.z},
				{m_AABB.min.x, m_AABB.max.y, m_AABB.min.z},
				{m_AABB.max.x, m_AABB.max.y, m_AABB.min.z},
				{m_AABB.min.x, m_AABB.min.y, m_AABB.max.z},
				{m_AABB.max.x, m_AABB.min.y, m_AABB.max.z},
				{m_AABB.min.x, m_AABB.max.y, m_AABB.max.z},
				{m_AABB.max.x, m_AABB.max.y, m_AABB.max.z}
			};

			float minProj = FLT_MAX;
			float maxProj = -FLT_MAX;
			for (const auto& c : corners)
			{
				const float proj = glm::dot(c, lightDir);
				minProj = std::min(minProj, proj);
				maxProj = std::max(maxProj, proj);
			}

			const float dst = maxProj - glm::dot(center, lightDir);
			const glm::vec3 lightPos = center - lightDir * dst;

			const glm::vec3 up = glm::abs(glm::dot(lightDir, glm::vec3(0.f, 1.f, 0.f))) < (1.f - FLT_EPSILON)
							   ? glm::vec3(0.f, 1.f, 0.f)
							   : glm::vec3(0.f, 0.f, -1.f);
			auto lookAt = glm::lookAtLH(lightPos, center, up);
			light.viewMatrices.resize(1);
			light.viewMatrices[0] = lookAt;

			glm::vec3 minLightSpace(FLT_MAX);
			glm::vec3 maxLightSpace(-FLT_MAX);
			for (const auto& c : corners)
			{
				const glm::vec3 transformedCorner = glm::vec3(lookAt * glm::vec4(c, 1.f));
				minLightSpace = glm::min(minLightSpace, transformedCorner);
				maxLightSpace = glm::max(maxLightSpace, transformedCorner);
			}

			constexpr float nearZ = 0.f;
			const float farZ = maxLightSpace.z - minLightSpace.z;
			light.projMatrix = glm::orthoLH(minLightSpace.x, maxLightSpace.x, minLightSpace.y, maxLightSpace.y, nearZ, farZ);
			light.projMatrix[1][1] *= -1.f;
		}
		// -- Point --
		else
		{
			glm::vec3 eye = glm::vec3(0.f);
			light.viewMatrices = {
				glm::lookAt(eye, eye + glm::vec3(1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // +X
				glm::lookAt(eye, eye + glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // -X
				glm::lookAt(eye, eye + glm::vec3(0.f, -1.f,  0.f), glm::vec3(0.f,  0.f, -1.f)), // -Y
				glm::lookAt(eye, eye + glm::vec3(0.f,  1.f,  0.f), glm::vec3(0.f,  0.f,  1.f)), // +Y
				glm::lookAt(eye, eye + glm::vec3(0.f,  0.f,  1.f), glm::vec3(0.f, -1.f,  0.f)), // +Z
				glm::lookAt(eye, eye + glm::vec3(0.f,  0.f, -1.f), glm::vec3(0.f, -1.f,  0.f)), // -Z
			};
			light.projMatrix = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 10.f);
		}
	}
}
void pom::Scene::GenerateDepthMaps(const Context& context, uint32_t size)
{
	CalculateLightMatrices();
	for (auto& l : m_vLights)
		l.GenerateDepthMap(context, this, size);
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
	AddLight(Light
		{
		/* direction */	{ -0.577f, -0.577f, -0.577f },
		/* color */		{ 1.f, 0.f, 1.f },
		/* lux */		1.f, Light::Type::Directional
		});
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


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Spheres Scene	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pom::SpheresScene::Initialize()
{
	AddModel("models/MetalRoughSpheres.gltf");
	SetEnvironmentMap("textures/circus_arena_4k.hdr");
}

