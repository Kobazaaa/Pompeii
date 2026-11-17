// -- Pompeii Includes --
#include "Light.h"

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include "Context.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"


//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Light
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::Light::CalculateLightMatrices(const AABB& aabb)
{
	if (type == LightType::Directional)
	{
		auto [min, max] = aabb;
		const glm::vec3 center = (min + max) * 0.5f;
		const glm::vec3 lightDir = dirPos;
		const std::vector<glm::vec3> corners = {
			{min.x, min.y, min.z},
			{max.x, min.y, min.z},
			{min.x, max.y, min.z},
			{max.x, max.y, min.z},
			{min.x, min.y, max.z},
			{max.x, min.y, max.z},
			{min.x, max.y, max.z},
			{max.x, max.y, max.z}
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
		viewMatrices.resize(1);
		viewMatrices[0] = lookAt;

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
		projMatrix = glm::orthoLH(minLightSpace.x, maxLightSpace.x, minLightSpace.y, maxLightSpace.y, nearZ, farZ);
		projMatrix[1][1] *= -1.f;
	}
	// -- Point --
	else
	{
		glm::vec3 eye = dirPos;
		viewMatrices = {
			glm::lookAt(eye, eye + glm::vec3(1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // +X
			glm::lookAt(eye, eye + glm::vec3(-1.f,  0.f,  0.f), glm::vec3(0.f, -1.f,  0.f)), // -X
			glm::lookAt(eye, eye + glm::vec3(0.f, -1.f,  0.f), glm::vec3(0.f,  0.f, -1.f)), // -Y
			glm::lookAt(eye, eye + glm::vec3(0.f,  1.f,  0.f), glm::vec3(0.f,  0.f,  1.f)), // +Y
			glm::lookAt(eye, eye + glm::vec3(0.f,  0.f,  1.f), glm::vec3(0.f, -1.f,  0.f)), // +Z
			glm::lookAt(eye, eye + glm::vec3(0.f,  0.f, -1.f), glm::vec3(0.f, -1.f,  0.f)), // -Z
		};
		projMatrix = glm::perspective(glm::radians(90.f), 1.f, 0.1f, 100.f);
	}
}

void pompeii::Light::CreateDepthImage(const Context& context, uint32_t size)
{
	if (!vShadowMaps.empty())
		DestroyDepthMap(context);

	// -- Build Depth Map Image on GPU --
	vShadowMaps.resize(context.maxFramesInFlight);
	for (Image& map : vShadowMaps)
	{
		ImageBuilder builder{};
		builder
			.SetDebugName("Light Depth Map")
			.SetWidth(size)
			.SetHeight(size)
			.SetFormat(VK_FORMAT_D32_SFLOAT)
			.SetTiling(VK_IMAGE_TILING_OPTIMAL)
			.SetUsageFlags(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
			.SetCreateFlags(type == LightType::Point ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0)
			.SetArrayLayers(type == LightType::Point ? 6 : 1)
			.SetMemoryProperties(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			.Build(context, map);

		map.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, type == LightType::Point ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D, 0, map.GetMipLevels(), 0, map.GetLayerCount());
		for (uint32_t i{}; i < (type == LightType::Point ? 6u : 1u); ++i)
			map.CreateView(context, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D, 0, 1, i, 1);
	}
}
void pompeii::Light::DestroyDepthMap(const Context& context)
{
	for (Image& image : vShadowMaps)
		image.Destroy(context);
	vShadowMaps.clear();
}
