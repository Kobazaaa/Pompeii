// -- Pompeii Includes --
#include "UIPass.h"
//#include "Window.h"
#include "Image.h"
#include "Context.h"
#include "Debugger.h"

// -- Standard Library --
#include <stdexcept>

// -- ImGui --
//#include "backends/imgui_impl_vulkan.h"
//#include "backends/imgui_impl_glfw.h"
//#include "ImGuiFileDialog.h"

//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
void pompeii::UIPass::Initialize(const Context& context, const UIPassCreateInfo& /*createInfo*/)
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
		//IMGUI_CHECKVERSION();
		//ImGui::CreateContext();
		//ImGuiIO& io = ImGui::GetIO();
		//io.IniFilename = nullptr;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		//SetupImGuiStyle();

		//// Setup Backends
		//ImGui_ImplGlfw_InitForVulkan(createInfo.pWindow->GetHandle(), true);

		//ImGui_ImplVulkan_InitInfo imGuiVulkanInitInfo = {};
		//imGuiVulkanInitInfo.Instance = context.instance.GetHandle();
		//imGuiVulkanInitInfo.PhysicalDevice = context.physicalDevice.GetHandle();
		//imGuiVulkanInitInfo.Device = context.device.GetHandle();
		//imGuiVulkanInitInfo.Queue = context.device.GetGraphicQueue();
		//imGuiVulkanInitInfo.DescriptorPool = m_DescriptorPool.GetHandle();
		//imGuiVulkanInitInfo.MinImageCount = context.maxFramesInFlight;
		//imGuiVulkanInitInfo.ImageCount = createInfo.swapchainImageCount;
		//imGuiVulkanInitInfo.UseDynamicRendering = true;
		//imGuiVulkanInitInfo.CheckVkResultFn = [](VkResult err)
		//	{
		//		if (err != VK_SUCCESS)
		//			throw std::runtime_error("ImGui Vulkan error");
		//	};
		//imGuiVulkanInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		//imGuiVulkanInitInfo.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
		//imGuiVulkanInitInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		//imGuiVulkanInitInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &createInfo.swapchainImageFormat;

		//ImGui_ImplVulkan_Init(&imGuiVulkanInitInfo);
		//m_DeletionQueue.Push([&]
		//	{
		//		ImGui_ImplVulkan_Shutdown();
		//		ImGui_ImplGlfw_Shutdown();
		//		ImGui::DestroyContext();
		//	});
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

void pompeii::UIPass::InsertUI(const std::function<void()>& func)
{
	m_vUICalls.push_back(func);
}

//--------------------------------------------------
//    Helpers
//--------------------------------------------------
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::BeginImGuiFrame() const
{
	//ImGui_ImplVulkan_NewFrame();
	//ImGui_ImplGlfw_NewFrame();
	//ImGui::NewFrame();
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::ImGuiLogic() const
{
	for (const auto& func : m_vUICalls)
		func();
}
// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::EndImGuiFrame(CommandBuffer& commandBuffer, const Image& renderImage)
{
	//const ImGuiIO& io = ImGui::GetIO();
	//ImGui::Render();
	//if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	//{
	//	auto backupWindowPtr = glfwGetCurrentContext();
	//	ImGui::UpdatePlatformWindows();
	//	ImGui::RenderPlatformWindowsDefault();
	//	glfwMakeContextCurrent(backupWindowPtr);
	//}

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
		//ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}
	vkCmdEndRendering(cmd);
	Debugger::EndDebugLabel(commandBuffer);
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void pompeii::UIPass::SetupImGuiStyle()
{
	//// -- Comfortable Dark Cyan style by SouthCraftX from ImThemes --
	//ImGuiStyle& style = ImGui::GetStyle();

	//// -- Styling --
	//style.Alpha								= 1.0f;
	//style.DisabledAlpha						= 1.0f;
	//style.WindowPadding						= ImVec2(20.0f, 20.0f);
	//style.WindowRounding					= 11.5f;
	//style.WindowBorderSize					= 0.0f;
	//style.WindowMinSize						= ImVec2(20.0f, 20.0f);
	//style.WindowTitleAlign					= ImVec2(0.5f, 0.5f);
	//style.WindowMenuButtonPosition			= ImGuiDir_None;
	//style.ChildRounding						= 20.0f;
	//style.ChildBorderSize					= 1.0f;
	//style.PopupRounding						= 17.39999961853027f;
	//style.PopupBorderSize					= 1.0f;
	//style.FramePadding						= ImVec2(20.0f, 3.400000095367432f);
	//style.FrameRounding						= 11.89999961853027f;
	//style.FrameBorderSize					= 0.0f;
	//style.ItemSpacing						= ImVec2(8.899999618530273f, 13.39999961853027f);
	//style.ItemInnerSpacing					= ImVec2(7.099999904632568f, 1.799999952316284f);
	//style.CellPadding						= ImVec2(12.10000038146973f, 9.199999809265137f);
	//style.IndentSpacing						= 21.0f;
	//style.ColumnsMinSpacing					= 8.699999809265137f;
	//style.ScrollbarSize						= 11.60000038146973f;
	//style.ScrollbarRounding					= 15.89999961853027f;
	//style.GrabMinSize						= 3.700000047683716f;
	//style.GrabRounding						= 20.0f;
	//style.TabRounding						= 9.800000190734863f;
	//style.TabBorderSize						= 0.0f;
	//style.TabCloseButtonMinWidthSelected	= 0.0f;
	//style.TabCloseButtonMinWidthUnselected	= 0.0f;
	//style.ColorButtonPosition				= ImGuiDir_Right;
	//style.ButtonTextAlign					= ImVec2(0.5f, 0.5f);
	//style.SelectableTextAlign				= ImVec2(0.0f, 0.0f);

	//// -- Colors --
	//style.Colors[ImGuiCol_Text]						= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	//style.Colors[ImGuiCol_TextDisabled]				= ImVec4(0.2745098173618317f, 0.3176470696926117f, 0.4509803950786591f, 1.0f);
	//style.Colors[ImGuiCol_WindowBg]					= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_ChildBg]					= ImVec4(0.09411764889955521f, 0.1019607856869698f, 0.1176470592617989f, 1.0f);
	//style.Colors[ImGuiCol_PopupBg]					= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_Border]					= ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	//style.Colors[ImGuiCol_BorderShadow]				= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_FrameBg]					= ImVec4(0.1137254908680916f, 0.125490203499794f, 0.1529411822557449f, 1.0f);
	//style.Colors[ImGuiCol_FrameBgHovered]			= ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	//style.Colors[ImGuiCol_FrameBgActive]			= ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	//style.Colors[ImGuiCol_TitleBg]					= ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	//style.Colors[ImGuiCol_TitleBgActive]			= ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	//style.Colors[ImGuiCol_TitleBgCollapsed]			= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_MenuBarBg]				= ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	//style.Colors[ImGuiCol_ScrollbarBg]				= ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	//style.Colors[ImGuiCol_ScrollbarGrab]			= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_ScrollbarGrabHovered]		= ImVec4(0.1568627506494522f, 0.168627455830574f, 0.1921568661928177f, 1.0f);
	//style.Colors[ImGuiCol_ScrollbarGrabActive]		= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_CheckMark]				= ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	//style.Colors[ImGuiCol_SliderGrab]				= ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	//style.Colors[ImGuiCol_SliderGrabActive]			= ImVec4(0.6000000238418579f, 0.9647058844566345f, 0.0313725508749485f, 1.0f);
	//style.Colors[ImGuiCol_Button]					= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_ButtonHovered]			= ImVec4(0.1803921610116959f, 0.1882352977991104f, 0.196078434586525f, 1.0f);
	//style.Colors[ImGuiCol_ButtonActive]				= ImVec4(0.1529411822557449f, 0.1529411822557449f, 0.1529411822557449f, 1.0f);
	//style.Colors[ImGuiCol_Header]					= ImVec4(0.1411764770746231f, 0.1647058874368668f, 0.2078431397676468f, 1.0f);
	//style.Colors[ImGuiCol_HeaderHovered]			= ImVec4(0.105882354080677f, 0.105882354080677f, 0.105882354080677f, 1.0f);
	//style.Colors[ImGuiCol_HeaderActive]				= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_Separator]				= ImVec4(0.1294117718935013f, 0.1490196138620377f, 0.1921568661928177f, 1.0f);
	//style.Colors[ImGuiCol_SeparatorHovered]			= ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	//style.Colors[ImGuiCol_SeparatorActive]			= ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	//style.Colors[ImGuiCol_ResizeGrip]				= ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1450980454683304f, 1.0f);
	//style.Colors[ImGuiCol_ResizeGripHovered]		= ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	//style.Colors[ImGuiCol_ResizeGripActive]			= ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	//style.Colors[ImGuiCol_Tab]						= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_TabHovered]				= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_TabActive]				= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_TabUnfocused]				= ImVec4(0.0784313753247261f, 0.08627451211214066f, 0.1019607856869698f, 1.0f);
	//style.Colors[ImGuiCol_TabUnfocusedActive]		= ImVec4(0.125490203499794f, 0.2745098173618317f, 0.572549045085907f, 1.0f);
	//style.Colors[ImGuiCol_PlotLines]				= ImVec4(0.5215686559677124f, 0.6000000238418579f, 0.7019608020782471f, 1.0f);
	//style.Colors[ImGuiCol_PlotLinesHovered]			= ImVec4(0.03921568766236305f, 0.9803921580314636f, 0.9803921580314636f, 1.0f);
	//style.Colors[ImGuiCol_PlotHistogram]			= ImVec4(0.0313725508749485f, 0.9490196108818054f, 0.843137264251709f, 1.0f);
	//style.Colors[ImGuiCol_PlotHistogramHovered]		= ImVec4(0.1568627506494522f, 0.1843137294054031f, 0.250980406999588f, 1.0f);
	//style.Colors[ImGuiCol_TableHeaderBg]			= ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	//style.Colors[ImGuiCol_TableBorderStrong]		= ImVec4(0.0470588244497776f, 0.05490196123719215f, 0.07058823853731155f, 1.0f);
	//style.Colors[ImGuiCol_TableBorderLight]			= ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	//style.Colors[ImGuiCol_TableRowBg]				= ImVec4(0.1176470592617989f, 0.1333333402872086f, 0.1490196138620377f, 1.0f);
	//style.Colors[ImGuiCol_TableRowBgAlt]			= ImVec4(0.09803921729326248f, 0.105882354080677f, 0.1215686276555061f, 1.0f);
	//style.Colors[ImGuiCol_TextSelectedBg]			= ImVec4(0.9372549057006836f, 0.9372549057006836f, 0.9372549057006836f, 1.0f);
	//style.Colors[ImGuiCol_DragDropTarget]			= ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	//style.Colors[ImGuiCol_NavHighlight]				= ImVec4(0.2666666805744171f, 0.2901960909366608f, 1.0f, 1.0f);
	//style.Colors[ImGuiCol_NavWindowingHighlight]	= ImVec4(0.4980392158031464f, 0.5137255191802979f, 1.0f, 1.0f);
	//style.Colors[ImGuiCol_NavWindowingDimBg]		= ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);
	//style.Colors[ImGuiCol_ModalWindowDimBg]			= ImVec4(0.196078434586525f, 0.1764705926179886f, 0.5450980663299561f, 0.501960813999176f);

	//// -- Base Font --
	//ImGuiIO& io = ImGui::GetIO();
	//io.Fonts->Clear();
	//io.Fonts->AddFontFromFileTTF("./fonts/JetBrainsMono-Medium.ttf", 22.f);
	//io.FontDefault = io.Fonts->Fonts.back();
}
