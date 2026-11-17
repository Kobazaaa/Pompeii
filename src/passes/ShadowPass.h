#ifndef SHADOW_PASS_H
#define SHADOW_PASS_H

// -- Math Includes --
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "Pipeline.h"

// -- Forward Declarations --
namespace pompeii
{
	struct RenderItem;
	struct LightItem;
	struct LightGPU;
	struct Mesh;
	struct Light;
	class CommandBuffer;
	struct RenderLightContext;
	struct RenderDrawContext;
	template <typename T> class ResourcePool;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Shadow Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class ShadowPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit ShadowPass() = default;
		~ShadowPass() = default;
		ShadowPass(const ShadowPass& other) = delete;
		ShadowPass(ShadowPass&& other) noexcept = delete;
		ShadowPass& operator=(const ShadowPass& other) = delete;
		ShadowPass& operator=(ShadowPass&& other) noexcept = delete;

		void Initialize(const Context& context);
		void Destroy();
		// lightPool is non-const: shadow map images have their tracked layout mutated during rendering.
		void Record(const Context& context, CommandBuffer& commandBuffer, const ResourcePool<Mesh>& meshPool, ResourcePool<Light>& lightPool,
			const std::vector<RenderItem>& renderItems, const std::vector<LightItem>& lightItems) const;


		//--------------------------------------------------
		//    Shader Infos
		//--------------------------------------------------
		struct alignas(16) PushConstants
		{
			glm::mat4 lightSpace;
			glm::mat4 model;
		};

	private:
		// -- Pipeline --
		PipelineLayout	m_ShadowPipelineLayout	{ };
		Pipeline		m_ShadowPipeline		{ };

		// -- DQ --
		DeletionQueue	m_DeletionQueue			{ };
	};
}
#endif // SHADOW_PASS_H