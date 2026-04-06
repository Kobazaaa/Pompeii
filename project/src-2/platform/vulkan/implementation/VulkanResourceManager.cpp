// -- Pompeii::vulkan Includes --
#include "VulkanResourceManager.h"

// -- Vulkan Includes --
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanShader.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
pompeii::vulkan::VulkanResourceManager::VulkanResourceManager(VulkanContext& context)
	: m_pContext(&context)
{
	m_CommandPool = VulkanCommandPool(context);
}


//--------------------------------------------------
//    Functionality
//--------------------------------------------------
std::unique_ptr<pompeii::IImage> pompeii::vulkan::VulkanResourceManager::CreateImage(const ImageDesc& desc)
{
	return std::make_unique<VulkanImage>(desc, *m_pContext);
}
void pompeii::vulkan::VulkanResourceManager::UploadImage(IImage& image, const void* data, size_t size)
{
	const auto cmd = m_CommandPool.Allocate(CommandBufferLevel::Primary);
	cmd->Begin(true);
	cmd->UploadDataToImage(image, data, size);
	cmd->End();
	cmd->Submit();
}

std::unique_ptr<pompeii::IBuffer> pompeii::vulkan::VulkanResourceManager::CreateBuffer(const BufferDesc& desc)
{
	return std::make_unique<VulkanBuffer>(desc, *m_pContext);
}
void pompeii::vulkan::VulkanResourceManager::UploadBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset)
{
	const auto cmd = m_CommandPool.Allocate(CommandBufferLevel::Primary);
	cmd->Begin(true);
	cmd->UploadDataToBuffer(buffer, data, size, offset);
	cmd->End();
	cmd->Submit();
}

std::unique_ptr<pompeii::IShader> pompeii::vulkan::VulkanResourceManager::CreateShader(const ShaderDesc& desc)
{
	return std::make_unique<VulkanShader>(*m_pContext, desc);
}

