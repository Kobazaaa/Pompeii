#ifndef GENERAL_LIGHT_H
#define GENERAL_LIGHT_H

// -- Standard Library --
#include <vector>

// -- Math Includes --
#include "glm/glm.hpp"

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  GPU Representation Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct alignas(16) GPULight
	{
		glm::vec4 dirPosType;
		glm::vec3 color;
		float intensity;
		uint32_t shadowMatrixOffset;
		uint32_t shadowMatrixCount;
		float _padding[2];
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct AABB;
	class Light final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		enum class Type
		{
			Directional, Point
		};
		Light() = default;
		Light(const glm::vec3& dirPos, const glm::vec3& col, float luxLumen, Type type);

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		Type GetType() const;

		glm::vec3 GetDirPos() const;
		void SetDirPos(const glm::vec3& dirPos);

		glm::vec3 GetColor() const;
		void SetColor(const glm::vec3& col);
		
		float GetLuxLumen() const;
		void SetLuxLumen(float luxLumen);

		void CalculateMatrices(const AABB& aabb);
		const std::vector<glm::mat4>& GetViewMatrices() const;
		const glm::mat4& GetProjectionMatrix() const;

	private:
		// -- Data --
		Type m_Type;
		glm::vec3 m_DirPos;
		glm::vec3 m_Color;
		float m_LuxLumen;

		// -- Matrices --
		std::vector<glm::mat4> m_ViewMatrix;
		glm::mat4 m_ProjMatrix;
	};
}

#endif // GENERAL_LIGHT_H