// -- Pompeii Includes --
#include "Shapes.h"

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  AABB	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::AABB::GrowToInclude(const glm::vec3& p)
{
	min = glm::min(min, p);
	max = glm::max(max, p);
}
void pompeii::AABB::GrowToInclude(const AABB& aabb)
{
	min = glm::min(min, aabb.min);
	max = glm::max(max, aabb.max);
}
