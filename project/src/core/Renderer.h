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
#include "DescriptorPool.h"
#include "FrameBuffer.h"
#include "Window.h"
#include "Model.h"
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
		std::vector<FrameBuffer>	m_vFrameBuffers			{ };
		uint32_t					m_MaxFramesInFlight		{ 3 };
		Image						m_MSAAImage				{ };

		// -- Model --
		Model						m_Model					{ };

		// -- Descriptors
		DescriptorPool				m_DescriptorPool		{ };

		// -- Command --
		CommandPool					m_CommandPool			{ };
		SyncManager					m_SyncManager			{ };

		// -- Passes --
		ShadowPass					m_ShadowPass			{ };
		ForwardPass					m_ForwardPass			{ };


		//--------------------------------------------------
		//    Helpers
		//--------------------------------------------------
		void RecreateSwapChain();
		void CreateFrameBuffers();
		void LoadModels();
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