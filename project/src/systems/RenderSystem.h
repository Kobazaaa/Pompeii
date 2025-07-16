#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "ISystem.h"

// -- Forward Declarations --
namespace pompeii
{
	class Model;
	class Camera;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Render System	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class RenderSystem final : public ISystem
	{
	public:
		//--------------------------------------------------
		//    Models
		//--------------------------------------------------
		void RegisterModel(Model& model);
		void UnregisterModel(const Model& model);
		const std::vector<Model*>& GetVisibleModels() const;
		const std::vector<Model*>& GetAllModels() const;

		//--------------------------------------------------
		//    Textures
		//--------------------------------------------------
		uint32_t GetTextureCount() const;

		//--------------------------------------------------
		//    Camera
		//--------------------------------------------------
		Camera* GetMainCamera() const;
		void SetMainCamera(Camera& camera);

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------
		void BeginFrame() override;
		void EndFrame() override;

	private:
		std::vector<Model*> m_vRegisteredModels{};
		std::vector<Model*> m_vVisibleModels{};
		Camera* m_pMainCamera{};

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void FrustumCull();
	};
}


#endif // RENDER_SYSTEM_H