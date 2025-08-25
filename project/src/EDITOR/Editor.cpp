// -- Pompeii Includes --
#include "Editor.h"

// -- ImGui --
#include "imgui_internal.h"


void pompeii::Editor::Draw() const
{
	// -- DockSpace --
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
		SetupDockSpace();

	// -- UI --
	m_pMenuBarUI->Draw();
	m_pSceneHierarchyUI->Draw(m_DockLeftID);
	m_pUtilitiesUI->Draw(m_DockLeftID);
	m_pInspectorUI->Draw(m_pSceneHierarchyUI->GetSelectedObject(), m_DockLeftBottomID);
}
void pompeii::Editor::SetupDockSpace() const
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const auto dockSpaceID = ImGui::DockSpaceOverViewport(viewport->ID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

	if (m_IsDockSpaceBuilt)
		return;

	ImGui::DockBuilderRemoveNode(dockSpaceID);
	ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockSpaceID, viewport->Size);

	m_DockCentralID = dockSpaceID;
	constexpr float ratio = 0.45f;
	m_DockLeftID = ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Left, ratio, nullptr, &m_DockCentralID);
	m_DockRightID = ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Right, ratio, nullptr, &m_DockCentralID);
	m_DockTopID = ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Up, ratio, nullptr, &m_DockCentralID);
	m_DockBottomID = ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Down, ratio, nullptr, &m_DockCentralID);

	m_DockLeftBottomID = ImGui::DockBuilderSplitNode(m_DockLeftID, ImGuiDir_Down, 0.75f, nullptr, &m_DockLeftID);

	ImGui::DockBuilderFinish(dockSpaceID);
	m_IsDockSpaceBuilt = true;
}
