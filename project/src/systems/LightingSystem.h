#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

// -- Standard Library --
#include <vector>

// -- Math --
#include "glm/fwd.hpp"

// -- Pompeii Includes --
#include "ISystem.h"

// -- Forward Declarations --
namespace pompeii
{
	class Light;
	struct GPULight;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Lighting System	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class LightingSystem final : public ISystem
	{
	public:
		//--------------------------------------------------
		//    Lights
		//--------------------------------------------------
		void RegisterLight(Light& light);
		void UnregisterLight(const Light& light);
		const std::vector<Light*>& GetLights() const;

		std::vector<GPULight> GetLightsGPU() const;
		uint32_t GetLightsCount() const;
		std::vector<glm::mat4> GetLightMatrices() const;

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------

	private:
		std::vector<Light*> m_vRegisteredLights{};
	};
}


#endif // RENDER_SYSTEM_H