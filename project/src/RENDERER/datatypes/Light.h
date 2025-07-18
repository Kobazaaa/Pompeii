#ifndef LIGHT_DATA_TYPE_H
#define LIGHT_DATA_TYPE_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Shapes.h"

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// -- Pompeii Includes --
#include "Image.h"
using LightHandle = uint32_t;

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light Type
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	enum class LightType
	{
		Directional, Point
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light CPU
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct LightCPU
	{
		glm::vec3 dirPos;
		glm::vec3 color;
		float luxLumen;
		LightType type;

		std::vector<glm::mat4> viewMatrices;
		glm::mat4 projMatrix;
		void CalculateLightMatrices(const AABB& aabb);
	};
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light GPU
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct alignas(16) LightGPUData
	{
		glm::vec4 dirPosType;
		glm::vec3 color;
		float intensity;
		uint32_t matrixIndex;
		uint32_t depthIndex;
		float _padding[2];
	};
	struct LightGPU
	{
		LightGPUData data;
		LightType type;
		std::vector<glm::mat4> viewMatrices;
		glm::mat4 projMatrix;

		Image shadowMap;
		std::vector<ImageView> views;

		void GenerateDepthMap(const Context& context, uint32_t size);
		void DestroyDepthMap(const Context& context);

	private:
		void GenerateDepthMap(const Context& context, Image& outImage, std::vector<ImageView>& outViews, uint32_t size) const;
	};
}

#endif // LIGHT_DATA_TYPE_H