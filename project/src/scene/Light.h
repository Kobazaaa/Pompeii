#ifndef GENERAL_LIGHT_H
#define GENERAL_LIGHT_H

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
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Light	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
		const GPULight& GetGPULight() const;
		Type GetType() const;

		glm::vec3 GetDirPos() const;
		void SetDirPos(const glm::vec3& dirPos);

		glm::vec3 GetColor() const;
		void SetColor(const glm::vec3& col);
		
		float GetLuxLumen() const;
		void SetLuxLumen(float luxLumen);

	private:
		// -- Data --
		Type m_Type;
		glm::vec3 m_DirPos;
		glm::vec3 m_Color;
		float m_LuxLumen;

		GPULight m_GPULight;
	};
}

#endif // GENERAL_LIGHT_H