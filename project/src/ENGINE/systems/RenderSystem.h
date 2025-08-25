#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

// -- Standard Library --
#include <vector>
#include <memory>

// -- Pompeii Includes --
#include "ISystem.h"
#include "Renderer.h"

// -- Forward Declarations --
namespace pompeii
{
	class MeshRenderer;
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
		void RegisterMeshRenderer(MeshRenderer& model);
		void UnregisterMeshRenderer(const MeshRenderer& model);

		//--------------------------------------------------
		//    Camera
		//--------------------------------------------------
		Camera* GetMainCamera() const;
		void SetMainCamera(Camera& camera);

		//--------------------------------------------------
		//    Interface
		//--------------------------------------------------
		void SetRenderer(const std::shared_ptr<Renderer>& renderer);
		Renderer* GetRenderer() const;
		void BeginFrame() override;
		void Update() override;
		void EndFrame() override;

	private:
		std::vector<MeshRenderer*> m_vPendingModels{};
		std::vector<MeshRenderer*> m_vRegisteredModels{};
		std::vector<MeshRenderer*> m_vVisibleModels{};
		Camera* m_pMainCamera{};
		std::shared_ptr<Renderer> m_pRenderer{};
		bool m_UpdateModels{};

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void FrustumCull();
		void AddPendingObjects();
		void UpdateData();
	};
}

#endif // RENDER_SYSTEM_H
