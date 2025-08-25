// -- Pompeii Includes --
#include "EditorUI.h"
#include "ServiceLocator.h"
#include "MeshRenderer.h"
#include "LightComponent.h"
#include "AssetManager.h"

// -- Standard Library --
#include <iostream>

// -- ImGui --
#include "ImGuiFileDialog.h"

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Interface	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::IEditorUI::HandleFileDialog(const std::string& key, const std::function<void(const std::string&)>& func)
{
    constexpr ImGuiWindowFlags fileDialogFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
    constexpr ImVec2 fileDialogSize = ImVec2(800, 500);
    if (ImGuiFileDialog::Instance()->Display(key, fileDialogFlags, fileDialogSize))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            const std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
            func(path);
        }
        ImGuiFileDialog::Instance()->Close();
    }
}

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Scene Hierarchy	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::SceneHierarchy::Draw(ImGuiID dockID)
{
    ImGui::SetNextWindowDockID(dockID, ImGuiCond_Once);
    ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_None);

    m_Index = 0;
    for (SceneObject* obj : ServiceLocator::Get<SceneManager>().GetActiveScene().GetAllObjects())
        if (!obj->transform->GetParent())
            DrawSceneObjectNode(obj);

    ImGui::End();
}
pompeii::SceneObject* pompeii::SceneHierarchy::GetSelectedObject() const
{
	return m_pSelectedObject;
}

void pompeii::SceneHierarchy::DrawSceneObjectNode(SceneObject* sceneObj)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
							   ImGuiTreeNodeFlags_OpenOnArrow |
							   ImGuiTreeNodeFlags_FramePadding |
							   ImGuiTreeNodeFlags_SpanAvailWidth;
    if (sceneObj->transform->GetAllChildren().empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    ImGui::PushID(m_Index);
    const bool nodeOpen = ImGui::TreeNodeEx(sceneObj->name.c_str(), flags, "%s", sceneObj->name.c_str());
    ImGui::PopID();
    ++m_Index;

    if (ImGui::IsItemClicked())
        m_pSelectedObject = sceneObj;

    // Right-click context menu
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Delete"))
        {
            if (sceneObj == m_pSelectedObject) m_pSelectedObject = nullptr;
        	sceneObj->Destroy();
        }
        ImGui::EndPopup();
    }

    // Drag source
    if (ImGui::BeginDragDropSource())
    {
        const SceneObject* payload = sceneObj;
        ImGui::SetDragDropPayload("SceneHierarchy", payload, sizeof(*payload));
        ImGui::Text("%s", sceneObj->name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drop target
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchy"))
        {
            if (payload->DataSize == sizeof(SceneObject))
            {
                const SceneObject* dropped = reinterpret_cast<const SceneObject*>(payload->Data);
                if (dropped && dropped != sceneObj)
                    dropped->transform->SetParent(sceneObj->transform.get(), true);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (nodeOpen)
    {
        for (const Transform* child : sceneObj->transform->GetAllChildren())
            DrawSceneObjectNode(child->GetSceneObject());
        ImGui::TreePop();
    }
}

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Menu Bar	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::MenuBar::Draw(ImGuiID)
{
    if (ImGui::BeginMainMenuBar())
    {
        // -- Config --
        if (ImGui::BeginMenu("Config"))
        {
            if (ImGui::MenuItem("Save Layout As..."))
                ImGuiFileDialog::Instance()->OpenDialog("SaveLayout", "Save Layout", ".ini");
            if (ImGui::MenuItem("Load Layout..."))
                ImGuiFileDialog::Instance()->OpenDialog("LoadLayout", "Load Layout", ".ini");

            ImGui::EndMenu();
        }

        // -- Dump VMA --
        if (ImGui::BeginMenu("DumpVMA"))
        {
            ImGuiFileDialog::Instance()->OpenDialog("DumpVMA", "Dump VMA", ".json");
            ImGui::EndMenu();
        }

        // -- VRAM --
        if (ImGui::BeginMenu("VRAM"))
        {
            VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
            vmaGetHeapBudgets(ServiceLocator::Get<RenderSystem>().GetRenderer()->GetContext().allocator, budgets);

            VkPhysicalDeviceMemoryProperties memProps{};
            vkGetPhysicalDeviceMemoryProperties(
                ServiceLocator::Get<RenderSystem>().GetRenderer()->GetContext().physicalDevice.GetHandle(),
                &memProps
            );

            for (uint32_t i = 0; i < memProps.memoryHeapCount; i++)
            {
                const VmaBudget& b = budgets[i];
                if (b.budget == 0) continue;

                const double usedMB = static_cast<double>(b.usage) / (1024.0 * 1024.0);
                const double budgetMB = static_cast<double>(b.budget) / (1024.0 * 1024.0);

                char buf[128];
                if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
                    snprintf(buf, sizeof(buf), "Heap %u (Device local): %.1f / %.1f MB", i, usedMB, budgetMB);
                else
                    snprintf(buf, sizeof(buf), "Heap %u: %.1f / %.1f MB", i, usedMB, budgetMB);

                ImGui::MenuItem(buf, nullptr, false, false);

                double appUsedMB = double(b.statistics.allocationBytes) / (1024.0 * 1024.0);
                double reservedMB = double(b.statistics.blockBytes) / (1024.0 * 1024.0);

                snprintf(buf, sizeof(buf), "Heap %u: App %.1f MB / Reserved %.1f MB", i, appUsedMB, reservedMB);
                ImGui::MenuItem(buf, nullptr, false, false);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // -- File Dialog Handles --
    HandleFileDialog("SaveLayout",  [](const std::string& path) { ImGui::SaveIniSettingsToDisk(path.c_str()); });
    HandleFileDialog("LoadLayout",  [](const std::string& path) { ImGui::LoadIniSettingsFromDisk(path.c_str()); });
    HandleFileDialog("DumpVMA",     [](const std::string& path)
        { 
	        char* StatsString = nullptr;
	        vmaBuildStatsString(ServiceLocator::Get<RenderSystem>().GetRenderer()->GetContext().allocator, &StatsString, true);
	        {
	            std::ofstream OutStats{ path };
	            OutStats << StatsString;
	        }
	        vmaFreeStatsString(ServiceLocator::Get<RenderSystem>().GetRenderer()->GetContext().allocator, StatsString);
        });
}

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Utilities	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::Utilities::Draw(ImGuiID dockID)
{
    ImGui::SetNextWindowDockID(dockID, ImGuiCond_Once);
    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_None);
    {
        ImGui::Text("Load Model");
        if (ImGui::Button("Choose Model"))
            ImGuiFileDialog::Instance()->OpenDialog("ChooseModel", "Choose a File", ".gltf");
        HandleFileDialog("ChooseModel", [](const std::string& path)
        {
			Mesh* mesh = ServiceLocator::Get<AssetManager>().LoadMesh(path);
            auto& obj = ServiceLocator::Get<SceneManager>().GetActiveScene().AddEmpty();
            auto filter = obj.AddComponent<MeshFilter>();
            filter->pMesh = mesh;
            obj.AddComponent<MeshRenderer>(*filter);
        });

        ImGui::Separator();

        ImGui::Text("Add Light");
        if (ImGui::Button("Add Light"))
        {
            auto& obj = ServiceLocator::Get<SceneManager>().GetActiveScene().AddEmpty();
            obj.AddComponent<LightComponent>(
                /* direction */	glm::vec3{ 0.577f, -0.577f, 0.577f },
                /* color */		glm::vec3{ 0.f, 1.f, 0.f },
                /* lux */			10.f, LightType::Directional
            );
        }
    }
    ImGui::End();
}

//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//? ~~	  Inspector	
//? ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void pompeii::Inspector::Draw(SceneObject* pSelectedObject, ImGuiID dockID)
{
    m_pSelectedObject = pSelectedObject;
    if (m_pSelectedObject)
        Draw(dockID);
}
void pompeii::Inspector::Draw(ImGuiID dockID)
{
    if (!m_pSelectedObject) return;
    ImGui::SetNextWindowDockID(dockID, ImGuiCond_Once);
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_None);
    m_pSelectedObject->OnInspectorDraw();
	ImGui::End();
}
