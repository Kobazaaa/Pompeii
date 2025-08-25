#ifndef LIGHT_DATA_TYPE_H
#define LIGHT_DATA_TYPE_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Shapes.h"
#include "Image.h"

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
	//? ~~	  Light
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct Light
	{
		// -- Data --
		glm::vec3 dirPos;
		LightType type;
		glm::vec3 color;
		float luxLumen;

		// -- Matrices --
		std::vector<glm::mat4> viewMatrices;
		glm::mat4 projMatrix;
		void CalculateLightMatrices(const AABB& aabb);

		// -- Shadow --
		std::vector<Image> vShadowMaps{};
		void CreateDepthImage(const Context& context, uint32_t size);
		void DestroyDepthMap(const Context& context);
	};
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light GPU
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct alignas(16) LightData
	{
		glm::vec3 dirPos;
		LightType type;
		glm::vec3 color;
		float intensity;
		glm::mat4 matrices[6]; //todo this wastes a lot of memory for directional lights, but we can fix this later
		uint32_t depthIndex;
		float _padding[3];
	};
}

#endif // LIGHT_DATA_TYPE_H