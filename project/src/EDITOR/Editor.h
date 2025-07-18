#ifndef EDITOR_H
#define EDITOR_H

// -- Standard Library --
#include <memory>

// -- Pompeii Includes --
#include "EditorUI.h"

namespace pompeii
{
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//? ~~	  Editor	
	//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	class Editor
	{
	public:
		void Draw();
	private:
		void SetupDockSpace() const;

		// -- Data --
		mutable bool m_IsDockSpaceBuilt{ false };
		mutable ImGuiID m_DockCentralID = 0;
		mutable ImGuiID m_DockLeftID = 0;
		mutable ImGuiID m_DockRightID = 0;
		mutable ImGuiID m_DockTopID = 0;
		mutable ImGuiID m_DockBottomID = 0;
		mutable ImGuiID m_DockLeftBottomID = 0;

		std::unique_ptr<SceneHierarchy> m_pSceneHierarchyUI{ std::make_unique<SceneHierarchy>() };
		std::unique_ptr<MenuBar> m_pMenuBarUI{ std::make_unique<MenuBar>() };
		std::unique_ptr<Utilities> m_pUtilitiesUI{ std::make_unique<Utilities>() };
		std::unique_ptr<Inspector> m_pInspectorUI{ std::make_unique<Inspector>() };
	};
}

#endif // EDITOR_H

