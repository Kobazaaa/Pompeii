#ifndef UI_PASS_H
#define UI_PASS_H

// -- Pompeii Includes --
#include "DeletionQueue.h"
#include "DescriptorPool.h"

// -- ImGui --
#include "imgui.h"

// -- Forward Declarations --
namespace pom
{
	class Image;
	class CommandBuffer;
	class Window;
}

namespace pom
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Create Info	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	struct UIPassCreateInfo
	{
		uint32_t maxFramesInFlight{};
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

	private:
		// -- Helpers --
		void BeginImGuiFrame();
		void ImGuiLogic();
		void EndImGuiFrame(CommandBuffer& commandBuffer, const Image& renderImage);

		void SetupDockSpace() const;
		void SetupImGuiStyle();

		// -- Data --
		mutable bool m_IsDockSpaceBuilt{ false };
		mutable ImGuiID m_DockCentralID = 0;
		mutable ImGuiID m_DockLeftID = 0;
		mutable ImGuiID m_DockRightID = 0;
		mutable ImGuiID m_DockTopID = 0;
		mutable ImGuiID m_DockBottomID = 0;

		// -- Pools --
		DescriptorPool m_DescriptorPool{};

		// -- DQ --
		DeletionQueue m_DeletionQueue{ };
	};
}
#endif // UI_PASS_H