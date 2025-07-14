// -- Pompeii Includes --
#include "UIPass.h"
#include "Window.h"
#include "Image.h"
#include "Context.h"
#include "Debugger.h"

// -- Standard Library --
#include <stdexcept>

// -- ImGui --
#include "imgui_internal.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "ImGuiFileDialog.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::UIPass::Initialize(const Context& context, const UIPassCreateInfo& createInfo)
{
	// -- Allocate ImGui Vulkan Descriptor Pool --
	{
		m_DescriptorPool
			.SetDebugName("ImGui Descriptor Pool")
			.AddFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.SetMaxSets(1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
			.Create(context);
		m_DeletionQueue.Push([&] { m_DescriptorPool.Destroy(context); });
	}

	// -- Init ImGui --
	{
		// Setup ImGui Context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = nullptr;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		SetupImGuiStyle();

		// Setup Backends
		ImGui_ImplGlfw_InitForVulkan(createInfo.pWindow->GetHandle(), true);

		ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {};
		imGuiVulkanInitInfo.Instance = context.instance.GetHandle();
		imGuiVulkanInitInfo.PhysicalDevice = context.physicalDevice.GetHandle();
		imGuiVulkanInitInfo.Device = context.device.GetHandle();
		imGuiVulkanInitInfo.Queue = context.device.GetGraphicQueue();
		imGuiVulkanInitInfo.DescriptorPool = m_DescriptorPool.GetHandle();
		imGuiVulkanInitInfo.MinImageCount = createInfo.maxFramesInFlight;
		imGuiVulkanInitInfo.ImageCount = createInfo.swapchainImageCount;
		imGuiVulkanInitInfo.UseDynamicRendering = true;
		imGuiVulkanInitInfo.CheckVkResultFn = [](VkResult err)
			{
				if (err != VK_SUCCESS)
					throw std::runtime_error("ImGui Vulkan error");
			};
		imGuiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		imGuiVulkanInitInfo.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		imGuiVulkanInitInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		imGuiVulkanInitInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &createInfo.swapchainImageFormat;

		ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo);
		m_DeletionQueue.Push([&]
			{
				ImGui_ImplVulkan_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
			});
	}
}
void pompeii::UIPass::Destroy()
{
	m_DeletionQueue.Flush();
}


//--------------------------------------------------
//    Record
//--------------------------------------------------
void pompeii::UIPass::Record(CommandBuffer& commandBuffer, const Image& renderImage)
{
	// -- New Frame Setup --
	BeginImGuiFrame();

	// -- ImGui Logic --
	ImGuiLogic();

	// -- End Frame --
	EndImGuiFrame(commandBuffer, renderImage);
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::BeginImGuiFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
		SetupDockSpace();
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::ImGuiLogic()
{
	// -- Menu --
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Config"))
		{
			if (ImGui::MenuItem("Save Layout As..."))
				ImGuiFileDialog::Instance()->OpenDialog("SaveLayout", "Save Layout", ".ini");
			if (ImGui::MenuItem("Load Layout..."))
				ImGuiFileDialog::Instance()->OpenDialog("LoadLayout", "Load Layout", ".ini");

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// -- FileDialog --
	constexpr ImGuiWindowFlags fileDialogFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking;
	constexpr ImVec2 fileDialogSize = ImVec2(800, 500);
	if (ImGuiFileDialog::Instance()->Display("SaveLayout", fileDialogFlags, fileDialogSize))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
			ImGui::SaveIniSettingsToDisk(path.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
	}
	if (ImGuiFileDialog::Instance()->Display("LoadLayout", fileDialogFlags, fileDialogSize))
	{
		if (ImGuiFileDialog::Instance()->IsOk())
		{
			const std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
			ImGui::LoadIniSettingsFromDisk(path.c_str());
		}
		ImGuiFileDialog::Instance()->Close();
	}

	// -- Sidebar --

	ImGui::SetNextWindowDockID(m_DockLeftID, ImGuiCond_Once);
	ImGui::Begin("I should be docked left", nullptr, ImGuiWindowFlags_None);
	ImGui::Text("I am docked left!");
	ImGui::End();
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::EndImGuiFrame(CommandBuffer& commandBuffer, const Image& renderImage)
{
	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Render();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	// -- Setup Attachment --
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = renderImage.GetView().GetHandle();
	colorAttachment.imageLayout = renderImage.GetCurrentLayout();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// -- Rendering Info --
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea = VkRect2D{ VkOffset2D{0, 0}, renderImage.GetExtent2D() };
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;

	// -- Render --
	const VkCommandBuffer cmd = commandBuffer.GetHandle();
	Debugger::BeginDebugLabel(commandBuffer, "UI Pass", glm::vec4(0.6f, 0.2f, 0.8f, 1));
	vkCmdBeginRendering(cmd, &renderingInfo);
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}
	vkCmdEndRendering(cmd);
	Debugger::EndDebugLabel(commandBuffer);
}

void pompeii::UIPass::SetupDockSpace() const
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	const auto dockSpaceID = ImGui::DockSpaceOverViewport(viewport->ID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

	if (m_IsDockSpaceBuilt)
		return;

	ImGui::DockBuilderRemoveNode(dockSpaceID);
	ImGui::DockBuilderAddNode(dockSpaceID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockSpaceID, viewport->Size);

	m_DockCentralID = dockSpaceID;
	constexpr float ratio = 0.35f;
	m_DockLeftID	= ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Left,  ratio, nullptr, &m_DockCentralID);
	m_DockRightID	= ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Right, ratio, nullptr, &m_DockCentralID);
	m_DockTopID		= ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Up,	   ratio, nullptr, &m_DockCentralID);
	m_DockBottomID	= ImGui::DockBuilderSplitNode(m_DockCentralID, ImGuiDir_Down,  ratio, nullptr, &m_DockCentralID);

	ImGui::DockBuilderFinish(dockSpaceID);
	m_IsDockSpaceBuilt = true;
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::SetupImGuiStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// -- Base --
	colors[ImGuiCol_WindowBg]			= ImVec4(0.05f, 0.05f, 0.05f, 1.f);
	colors[ImGuiCol_Header]				= ImVec4(0.12f, 0.12f, 0.12f, 1.f );
	colors[ImGuiCol_HeaderHovered]		= ImVec4(0.16f, 0.16f, 0.16f, 1.f );
	colors[ImGuiCol_HeaderActive]		= ImVec4(0.2f , 0.2f , 0.2f , 1.f );
	colors[ImGuiCol_FrameBg]			= ImVec4(0.08f, 0.08f, 0.08f, 1.f );
	colors[ImGuiCol_FrameBgHovered]		= ImVec4(0.14f, 0.14f, 0.14f, 1.f );
	colors[ImGuiCol_FrameBgActive]		= ImVec4(0.18f, 0.18f, 0.18f, 1.f );
	colors[ImGuiCol_TitleBg]			= ImVec4(0.06f, 0.06f, 0.06f, 1.f );
	colors[ImGuiCol_TitleBgActive]		= ImVec4(0.10f, 0.1f , 0.1f , 1.f );
	colors[ImGuiCol_PopupBg]			= ImVec4(0.08f, 0.08f, 0.08f, 1.f );
	colors[ImGuiCol_Border]				= ImVec4(0.18f, 0.18f, 0.18f, 1.f );
	colors[ImGuiCol_Separator]			= ImVec4(0.25f, 0.25f, 0.25f, 1.f );

	// -- Accent --
	constexpr ImVec4 accent			= ImVec4(0.16f, 0.45f, 0.32f, 1.f);
	constexpr ImVec4 accentHovered	= ImVec4(0.2f , 0.55f, 0.4f , 1.f);
	constexpr ImVec4 accentActive	= ImVec4(0.12f, 0.35f, 0.25f, 1.f);

	colors[ImGuiCol_Button]					= accent;
	colors[ImGuiCol_ButtonHovered]			= accentHovered;
	colors[ImGuiCol_ButtonActive]			= accentActive;
	colors[ImGuiCol_SliderGrab]				= accent;
	colors[ImGuiCol_SliderGrabActive]		= accentHovered;
	colors[ImGuiCol_CheckMark]				= accent;

	colors[ImGuiCol_Tab]					= accent;
	colors[ImGuiCol_TabHovered]				= accentHovered;
	colors[ImGuiCol_TabActive]				= accentActive;
	colors[ImGuiCol_TabUnfocused]			= ImVec4(0.1f, 0.1f, 0.1f, 1.f);
	colors[ImGuiCol_TabUnfocusedActive]		= accentActive;

	colors[ImGuiCol_TitleBgActive]			= ImVec4(0.1f, 0.25f, 0.2f, 1.f);
	colors[ImGuiCol_ResizeGrip]				= accent;
	colors[ImGuiCol_ResizeGripHovered]		= accentHovered;
	colors[ImGuiCol_ResizeGripActive]		= accentActive;

	colors[ImGuiCol_PlotLinesHovered]		= accentHovered;
	colors[ImGuiCol_PlotHistogram]			= accent;
	colors[ImGuiCol_PlotHistogramHovered]	= accentHovered;
	colors[ImGuiCol_DockingPreview]			= accent;

	// -- Tweaks --
	constexpr float roundedness = 4.f;
	style.FrameRounding		= roundedness;
	style.WindowRounding	= roundedness;
	style.ScrollbarRounding = roundedness;
	style.GrabRounding		= roundedness;
	style.TabRounding		= roundedness;

	style.WindowBorderSize = 2.f;

	// -- Base Font --
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("./fonts/JetBrainsMono-Medium.ttf", 22.f);
	io.FontDefault = io.Fonts->Fonts.back();
}
