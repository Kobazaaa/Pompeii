#ifndef LIGHTING_SYSTEM_H
#define LIGHTING_SYSTEM_H

// -- Standard Library --
#include <vector>
#include <memory>

// -- Pompeii Includes --
#include "ISystem.h"
#include "Renderer.h"

// -- Forward Declarations --
namespace pompeii
{
	class ModelRenderer;
	class Camera;
	class LightComponent;
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
		void RegisterLight(LightComponent& light);
		void UnregisterLight(const LightComponent& light);

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------
		void SetRenderer(const std::shared_ptr<Renderer>& renderer);
		Renderer* GetRenderer() const;
		void BeginFrame() override;
		void Update() override;
		void EndFrame() override;

	private:
		std::vector<LightComponent*> m_vRegisteredLights{};
		std::shared_ptr<Renderer> m_pRenderer{};
	};
}


#endif // RENDER_SYSTEM_H