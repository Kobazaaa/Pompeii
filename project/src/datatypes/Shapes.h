#ifndef SHAPES_DATA_TYPE_H
#define SHAPES_DATA_TYPE_H

// -- Math --
#include "glm/glm.hpp"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  AABB	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct AABB
	{
		glm::vec3 min{ FLT_MAX };
		glm::vec3 max{ -FLT_MAX };

		void GrowToInclude(const glm::vec3& p);
		void GrowToInclude(const AABB& aabb);
	};
}

#endif // SHAPES_DATA_TYPE_H