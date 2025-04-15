#ifndef RENDER_PASS_H
#define RENDER_PASS_H

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Standard Library --
#include <vector>

// -- Forward Declarations --
namespace pom { struct Context; }


namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  RenderPass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class RenderPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		RenderPass() = default;
		void Destroy(const Context& context) const;

		//--------------------------------------------------
		//    Accessors & Mutators
		//--------------------------------------------------
		const VkRenderPass& GetHandle() const;

	private:
		VkRenderPass m_RenderPass;
		friend class RenderPassBuilder;
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  RenderPassBuilder
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class RenderPassBuilder final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		RenderPassBuilder() = default;


		//--------------------------------------------------
		//    Builder
		//--------------------------------------------------
		RenderPassBuilder& NewSubpass();
		RenderPassBuilder& SetBindPoint(VkPipelineBindPoint bindPoint);

		RenderPassBuilder& NewAttachment();
		RenderPassBuilder& SetFormat(VkFormat format);
		RenderPassBuilder& SetSamples(VkSampleCountFlagBits samples);
		RenderPassBuilder& SetLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
		RenderPassBuilder& SetStencilLoadStoreOp(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
		RenderPassBuilder& SetInitialLayout(VkImageLayout layout);
		RenderPassBuilder& SetFinalLayout(VkImageLayout layout);
		RenderPassBuilder& AddSubpassColorAttachment(uint32_t attachment);
		RenderPassBuilder& SetSubpassResolveAttachment(uint32_t attachment);
		RenderPassBuilder& SetSubpassDepthAttachment(uint32_t attachment);

		RenderPassBuilder& NewDependency();
		RenderPassBuilder& SetSrcSubPass(uint32_t subpass);
		RenderPassBuilder& SetDstSubPass(uint32_t subpass);
		RenderPassBuilder& SetSrcMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags);
		RenderPassBuilder& SetDstMasks(VkPipelineStageFlags stageFlags, VkAccessFlags accessFlags);

		void Build(const Context& context, RenderPass& renderPass) const;

	private:
		std::vector<VkAttachmentDescription>	m_vAttachmentDescriptions{};
		std::vector<VkSubpassDescription>		m_vSubPasses{};
		std::vector<VkSubpassDependency>		m_vSubPassDependencies{};
		uint32_t								m_SubPassIdx{};

		std::vector<VkAttachmentReference>				m_ResolveAttachmentRef{ };
		std::vector<VkAttachmentReference>				m_vDepthAttachmentRefs{ };
		std::vector<std::vector<VkAttachmentReference>>	m_vColorAttachmentRefs{ };

	};
}

#endif // RENDER_PASS_H