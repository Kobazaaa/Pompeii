#ifndef POMPEII_VULKAN_RESOURCE_MANAGER_IMPLEMENTATION_H
#define POMPEII_VULKAN_RESOURCE_MANAGER_IMPLEMENTATION_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "IResourceManager.h"
#include "VulkanCommandPool.h"

// -- Forward Declaration --
namespace pompeii::vulkan
{
	struct VulkanContext;
}

namespace pompeii::vulkan
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~    VulkanResourceManager
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class VulkanResourceManager final : public IResourceManager
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit VulkanResourceManager() = default;
		explicit VulkanResourceManager(VulkanContext& context);
		~VulkanResourceManager() override = default;

		VulkanResourceManager(const VulkanResourceManager& other) = default;
		VulkanResourceManager(VulkanResourceManager&& other) noexcept = default;
		VulkanResourceManager& operator=(const VulkanResourceManager& other) = default;
		VulkanResourceManager& operator=(VulkanResourceManager&& other) noexcept = default;

		//--------------------------------------------------
		//    Functionality
		//--------------------------------------------------
		[[nodiscard]] std::unique_ptr<IImage> CreateImage(const ImageDesc& desc) override;
		void UploadImage(IImage& image, const void* data, size_t size) override;

		[[nodiscard]] std::unique_ptr<IBuffer> CreateBuffer(const BufferDesc& desc) override;
		void UploadBuffer(IBuffer& buffer, const void* data, size_t size, size_t offset) override;

		[[nodiscard]] std::unique_ptr<IShader> CreateShader(const ShaderDesc& desc) override;

	private:
		VulkanContext* m_pContext{ nullptr };
		VulkanCommandPool m_CommandPool;
	};
}

#endif // POMPEII_VULKAN_RESOURCE_MANAGER_IMPLEMENTATION_H
