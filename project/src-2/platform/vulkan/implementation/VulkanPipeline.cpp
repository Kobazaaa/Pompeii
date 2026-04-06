// -- Pompeii Includes --
#include "VulkanPipeline.h"
#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanHelpers.h"

// -- Standard Library --
#include <stdexcept>
#include <vector>

using namespace pompeii;
using namespace pompeii::vulkan;


//--------------------------------------------------
//    Constructor & Destructor
//--------------------------------------------------
VulkanPipeline::VulkanPipeline(VulkanContext& context, const PipelineDesc& desc)
    : m_pContext(&context)
{
    CreatePipelineLayout(desc);
	CreatePipeline(desc);
}

VulkanPipeline::~VulkanPipeline()
{
    if (m_Pipeline) vkDestroyPipeline(m_pContext->device, m_Pipeline, nullptr);
    if (m_Layout) vkDestroyPipelineLayout(m_pContext->device, m_Layout, nullptr);
}


//--------------------------------------------------
//    Operator Overloads
//--------------------------------------------------
VulkanPipeline::operator VkPipeline ()          const { return m_Pipeline; }
VulkanPipeline::operator VkPipelineLayout ()    const { return m_Layout; }


//--------------------------------------------------
//    Helpers
//--------------------------------------------------
void VulkanPipeline::CreatePipelineLayout(const PipelineDesc& desc)
{
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    layoutInfo.setLayoutCount = 0;
    layoutInfo.pSetLayouts = nullptr;

    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(m_pContext->device, &layoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
        throw std::runtime_error("failed to create Pipeline Layout!");
}

void VulkanPipeline::CreatePipeline(const PipelineDesc& desc)
{
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
    for (const auto& [binding, stride, inputRate] : desc.vertexLayout.bindings)
    {
        VkVertexInputBindingDescription b{};
        b.binding = binding;
        b.stride = stride;
        b.inputRate = TO_VK_VERTEX_INPUT_RATE(inputRate);
        bindingDescriptions.push_back(b);
    }

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    for (const auto& [location, binding, format, offset] : desc.vertexLayout.attributes)
    {
        VkVertexInputAttributeDescription a{};
        a.binding = binding;
        a.location = location;
        a.format = TO_VK_VERTEX_ATTRIBUTE_FORMAT(format);
        a.offset = offset;
        attributeDescriptions.push_back(a);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();



    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = TO_VK_PRIMITIVE_TOPOLOGY(desc.primitiveTopology);
    inputAssembly.primitiveRestartEnable = VK_FALSE;



    std::vector<VkDynamicState> dynamicStates = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();



    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;



    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = TO_VK_POLYGON_MODE(desc.polygonMode);
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = TO_VK_CULL_MODE(desc.cullMode);
    rasterizer.frontFace = TO_VK_FRONT_FACE(desc.frontFace);
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = VK_FALSE;
    rasterizer.depthBiasSlopeFactor = VK_FALSE;



    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.minSampleShading = 0.f;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;



    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = desc.depthRead ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = desc.depthWrite ? VK_TRUE : VK_FALSE;
    depthStencil.depthCompareOp = TO_VK_DEPTH_COMPARE_OP(desc.depthCompareOp);
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.f;
	depthStencil.maxDepthBounds = 1.f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};



    uint32_t formatCount = desc.colorAttachmentFormats.size();
    std::vector<VkFormat> formats{};
    formats.reserve(formatCount);
    for (auto format : desc.colorAttachmentFormats) formats.push_back(TO_VK_FORMAT(format));
    VkPipelineRenderingCreateInfo dynamicRendering{};
    dynamicRendering.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    dynamicRendering.colorAttachmentCount = formatCount;
    dynamicRendering.pColorAttachmentFormats = formats.data();
    if (desc.depthRead || desc.depthWrite) dynamicRendering.depthAttachmentFormat = TO_VK_FORMAT(desc.depthFormat);



    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
    for (uint32_t i{}; i < dynamicRendering.colorAttachmentCount; ++i)
    {
        colorBlendAttachments.push_back(
            {
                .blendEnable = VK_FALSE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
                .colorWriteMask =
                        VK_COLOR_COMPONENT_R_BIT |
                        VK_COLOR_COMPONENT_G_BIT |
                        VK_COLOR_COMPONENT_B_BIT |
                        VK_COLOR_COMPONENT_A_BIT
            }
        );
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size());;
    colorBlending.pAttachments = colorBlendAttachments.data();

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    if (desc.vertexShader)
    {
        auto& vkShader = static_cast<VulkanShader&>(*desc.vertexShader);
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = TO_VK_SHADER_STAGE(vkShader.GetStage());
        stageInfo.module = vkShader;
        stageInfo.pName = "main";
        shaderStages.push_back(stageInfo);
    }
    if (desc.fragmentShader)
    {
        auto& vkShader = static_cast<VulkanShader&>(*desc.fragmentShader);
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = TO_VK_SHADER_STAGE(vkShader.GetStage());
        stageInfo.module = vkShader;
        stageInfo.pName = "main";
        shaderStages.push_back(stageInfo);
    }


    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.renderPass = VK_NULL_HANDLE;
    pipelineInfo.layout = m_Layout;
    pipelineInfo.pNext = &dynamicRendering;

    if (vkCreateGraphicsPipelines(m_pContext->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
        throw std::runtime_error("Failed to create graphics pipeline!");
}
