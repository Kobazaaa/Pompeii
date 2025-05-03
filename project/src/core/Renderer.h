#ifndef RENDERER_H
#define RENDERER_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/quaternion.hpp"

// -- Standard Library --
#include <vector>

// -- Pompeii Includes --
#include "Context.h"
#include "CommandPool.h"
#include "DepthPrePass.h"
#include "DescriptorPool.h"
#include "Window.h"
#include "Scene.h"
#include "SwapChain.h"
#include "SyncManager.h"
#include "ForwardPass.h"
#include "ShadowPass.h"


// -- Forward Declarations --
namespace pom
{
	class Window;
	class Camera;
}

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Renderer	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Renderer final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		Renderer() = default;
		void Initialize(Camera* pCamera, Window* pWindow);
		void Destroy();

		//--------------------------------------------------
		//    Loop
		//--------------------------------------------------
		void Update();
		void Render();

	private:
		//--------------------------------------------------
		//    Vulkan Specific
		//--------------------------------------------------
		void InitializeVulkan();

		// -- Vulkan Context --
		Context						m_Context				{ };

		// -- SwapChain --
		SwapChain					m_SwapChain				{ };
		uint32_t					m_MaxFramesInFlight		{ 3 };

		// -- Pools --
		DescriptorPool				m_DescriptorPool		{ };
		CommandPool					m_CommandPool			{ };

		// -- Sync --
		SyncManager					m_SyncManager			{ };

		// -- Passes --
		ShadowPass					m_ShadowPass			{ };
		DepthPrePass				m_DepthPrePass			{ };
		ForwardPass					m_ForwardPass			{ };

		// -- Scene --
		Scene*						m_pScene{ };

		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void RecordCommandBuffer(CommandBuffer& commandBuffer, uint32_t imageIndex);

		// -- Window --
		Window*				m_pWindow			{ };
		Camera*				m_pCamera			{ };

		// -- Frame Counter --
		uint32_t			m_CurrentFrame		{ 0 };

		// -- Deletion Queues --
		DeletionQueue		m_DeletionQueueSC	{ };

	};
}

#endif // RENDERER_H