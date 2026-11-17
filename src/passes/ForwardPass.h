//#ifndef FORWARD_PASS_H
//#define FORWARD_PASS_H
//
//// -- Math Includes --
//#include "glm/glm.hpp"
//
//// -- Pompeii Includes --
//#include "DeletionQueue.h"
//#include "DescriptorSet.h"
//#include "Pipeline.h"
//#include "Sampler.h"
//#include "Image.h"
//
//// -- Forward Declarations --
//namespace pompeii
//{
//	class DescriptorPool;
//	class Scene;
//	class ShadowPass;
//	class Camera;
//	class CommandBuffer;
//}
//
//namespace pompeii
//{
//	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	//? ~~	  Create Info	
//	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	struct ForwardPassCreateInfo
//	{
//		uint32_t maxFramesInFlight{};
//		Scene* pScene{};
//		VkExtent2D extent{};
//		VkFormat format{};
//		VkFormat depthFormat{};
//		ShadowPass* pShadowPass{};
//	};
//
//
//	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	//? ~~	  Forward Pass	
//	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	class ForwardPass final
//	{
//	public:
//		//--------------------------------------------------
//		//    Constructor & Destructor
//		//--------------------------------------------------
//		explicit ForwardPass() = default;
//		~ForwardPass() = default;
//		ForwardPass(const ForwardPass& other) = delete;
//		ForwardPass(ForwardPass&& other) noexcept = delete;
//		ForwardPass& operator=(const ForwardPass& other) = delete;
//		ForwardPass& operator=(ForwardPass&& other) noexcept = delete;
//
//		void Initialize(const Context& context, const ForwardPassCreateInfo& createInfo);
//		void Destroy();
//		void Resize(const Context& context, VkExtent2D extent, VkFormat format);
//		void Record(const Context& context, CommandBuffer& commandBuffer, uint32_t imageIndex, Image& recordImage, Image& depthImage, Scene* pScene, Camera* pCamera);
//
//		//--------------------------------------------------
//		//    Shader Infos
//		//--------------------------------------------------
//		struct alignas(16) UniformBufferVS
//		{
//			glm::mat4 view;
//			glm::mat4 proj;
//			glm::mat4 lightSpace;
//		};
//		struct PCModelDataVS
//		{
//			glm::mat4 model;
//		};
//
//		struct alignas(16) UniformBufferFS
//		{
//			glm::vec3 dir;
//			float padding;
//			glm::vec3 color;
//			float intensity;
//		};
//		struct PCMaterialDataFS
//		{
//			// -- Textures --
//			uint32_t diffuseIdx;
//			uint32_t opacityIdx;
//			uint32_t specularIdx;
//			uint32_t shininessIdx;
//			uint32_t heightIdx;
//
//			// -- Data --
//			float expo;
//		};
//
//	private:
//		// -- Pipeline --
//		PipelineLayout		m_DefaultPipelineLayout	{ };
//		Pipeline			m_OpaquePipeline		{ };
//		Pipeline			m_TransparentPipeline	{ };
//
//		// -- Image --
//		Sampler						m_Sampler				{ };
//		Image						m_MSAAImage				{ };
//		void CreateMSAAImage(const Context& context, VkExtent2D extent, VkFormat format);
//
//		// -- Descriptors --
//		DescriptorSetLayout			m_UniformDSL			{ };
//		std::vector<DescriptorSet>	m_vUniformDS			{ };
//
//		DescriptorSetLayout			m_LightDSL				{ };
//		std::vector<DescriptorSet>	m_vLightDS				{ };
//
//		DescriptorSetLayout			m_ShadowMapDSL			{ };
//		std::vector<DescriptorSet>	m_ShadowMapDS			{ };
//
//		DescriptorSetLayout			m_TextureDSL			{ };
//		DescriptorSet				m_TextureDS				{ };
//
//		// -- Buffers --
//		std::vector<Buffer>			m_vUniformBuffers		{ };
//		std::vector<Buffer>			m_vLightBuffers			{ };
//
//		// -- DQ --
//		DeletionQueue				m_DeletionQueue			{ };
//	};
//}
//
//#endif // FORWARD_PASS_H