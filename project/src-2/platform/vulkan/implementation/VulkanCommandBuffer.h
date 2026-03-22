#ifndef POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H
#define POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H

// -- Pompeii Includes --
#include "ICommandBuffer.h"

// -- Vulkan Includes --
#include <vulkan/vulkan.h>

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
		explicit VulkanCommandBuffer(ICommandPool& commandPool);
		~VulkanCommandBuffer() override = default;

		//--------------------------------------------------
		//    Accessors
		//--------------------------------------------------
		ICommandPool* GetOwnerPool() const override;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		void Begin() override;
		void End() override;
		void Clear() override;

		//--------------------------------------------------
		//    Commands
		//--------------------------------------------------
		// Copy Commands
		void CopyBuffer(IBuffer* src, IBuffer* dst, size_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0) override;
		void CopyImage(IImage* src, IImage* dst) override;
		void CopyBufferToImage(IBuffer* src, IImage* dst) override;
		void Blit(IImage* src, IImage* dst) override;

		// Image Commands
		void GenerateMips(IImage* src) override;

		// Drawing
		void DrawMesh(Mesh* pMesh) override;

	private:
		ICommandPool* m_pCommandPool	{ nullptr };
		VkCommandBuffer m_CmdBuffer		{ VK_NULL_HANDLE };
		friend class VulkanCommandPool;
	};
}

#endif // POMPEII_VULKAN_COMMAND_BUFFER_IMPLEMENTATION_H
