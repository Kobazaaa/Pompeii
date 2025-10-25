#ifndef UI_PASS_H
#define UI_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"

// -- Forward Declarations --
namespace pompeii
{
	class Image;
	class CommandBuffer;
	class Window;
}

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct UIPassCreateInfo
	{
		uint32_t swapchainImageCount{};
		VkFormat swapchainImageFormat{};
		Window* pWindow{};
	};

	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  UI Pass	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class UIPass final
	{
	public:
		//--------------------------------------------------
		//    Constructor & Destructor
		//--------------------------------------------------
		explicit UIPass() = default;
		~UIPass() = default;
		UIPass(const UIPass& other) = delete;
		UIPass(UIPass&& other) noexcept = delete;
		UIPass& operator=(const UIPass& other) = delete;
		UIPass& operator=(UIPass&& other) noexcept = delete;

		void Initialize(const Context& context, const UIPassCreateInfo& createInfo);
		void Destroy();
		void Record(CommandBuffer& commandBuffer, const Image& renderImage);
		void InsertUI(const std::function<void()>& func);

	private:
		// -- Helpers --
		void BeginImGuiFrame() const;
		void ImGuiLogic() const;
		void EndImGuiFrame(CommandBuffer& commandBuffer, const Image& renderImage);

		void SetupImGuiStyle();

		// -- Pools --
		DescriptorPool m_DescriptorPool{};

		// -- Extra --
		DeletionQueue m_DeletionQueue{ };
		std::vector<std::function<void()>> m_vUICalls{};
	};
}
#endif // UI_PASS_H