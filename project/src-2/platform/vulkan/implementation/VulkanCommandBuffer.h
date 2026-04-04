#ifndef POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H
#define POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "ICommandBuffer.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

// -- Forward Declarations --
namespace pompeii::vulkan
{
	class VulkanImage;
	struct VulkanContext;
	class VulkanCommandPool;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanCommandBuffer
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanCommandBuffer final : public ICommandBuffer
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanCommandBuffer(VulkanCommandPool& commandPool, VulkanContext& context);
		~VulkanCommandBuffer() override = default;

		VulkanCommandBuffer(const VulkanCommandBuffer& other) = delete;
		VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept = default;
		VulkanCommandBuffer& operator=(const VulkanCommandBuffer& other) = delete;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&& other) noexcept = default;

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		ICommandPool* GetOwnerPool() const override;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void Begin(bool oneTime = false) override;
		void End() override;
		void Submit(FrameContext& frameCtx) override;
		void Submit() override;
		void Clear() override;

		//--------------------------------------------------
		//    Operation Overloads
		//--------------------------------------------------
		operator VkCommandBuffer() const;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		// Data Commands
		void UploadDataToBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset) override;
		void UploadDataToImage(IImage& image, const void* data, size_t size) override;

		// Copy Commands
		void CopyBuffer(IBuffer& src, IBuffer& dst, size_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) override;
		void CopyImage(IImage& src, IImage& dst) override;
		void CopyBufferToImage(IBuffer& src, IImage& dst) override;
		void Blit(IImage& src, IImage& dst) override;

		// Image Commands
		void GenerateMips(IImage& src) override;

		// Drawing
		void DrawMesh(Mesh& pMesh) override;

	private:
		void TransitionImageLayout(VulkanImage& image, VkImageLayout newLayout,
			VkAccessFlags2 srcAccess, VkPipelineStageFlags2 srcStage,
			VkAccessFlags2 dstAccess, VkPipelineStageFlags2 dstStage) const;

		VulkanContext* m_pContext			{ nullptr };
		VulkanCommandPool* m_pCommandPool	{ nullptr };
		VkCommandBuffer m_CmdBuffer			{ VK_NULL_HANDLE };
		bool m_IsTemporary					{ false };
		friend class VulkanCommandPool;
	};
}

#endif // POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H
