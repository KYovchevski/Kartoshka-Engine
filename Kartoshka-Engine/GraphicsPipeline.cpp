#include "GraphicsPipeline.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "RenderPass.h"
#include "DescriptorSetPool.h"
#include "DescriptorSetAllocation.h"

#include "VkHelpers.h"

VkPipelineVertexInputStateCreateInfo krt::VertexInputInfo::GetVkPipelineVertexInputStateCreateInfo()
{
    VkPipelineVertexInputStateCreateInfo output = {};

    static auto bindingList = std::vector<VkVertexInputBindingDescription>();
    bindingList.reserve(m_Bindings.size());
    for (auto& binding : m_Bindings)
    {
        bindingList.emplace_back(binding.second);
    }

    output.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    output.pVertexAttributeDescriptions = m_Attributes.data();
    output.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_Attributes.size());
    output.pVertexBindingDescriptions = bindingList.data();
    output.vertexBindingDescriptionCount = static_cast<uint32_t>(m_Bindings.size());

    return output;
}

uint32_t krt::VertexInputInfo::FindOffset(uint32_t a_Binding)
{
	uint32_t offset = 0;
	for (uint32_t i = 0; i < m_Attributes.size() - 1; i++)
	{
        if (m_Attributes[i].binding == a_Binding)
        {
            offset = m_Attributes[i].offset + m_Bindings[i].stride;
            break;
        }
	}

    return offset;
}

void krt::VertexInputInfo::CorrectStrides(uint32_t a_Binding, uint32_t a_NewStride)
{
    for (uint32_t i = 0; i < m_Bindings.size(); i++)
    {
        auto& binding = m_Bindings[i];
        if (binding.binding == a_Binding)
        {
            binding.stride += a_NewStride;
        }
    }
}

krt::PrimitiveAssemblyInfo krt::PrimitiveAssemblyInfo::CreateDefault()
{
    PrimitiveAssemblyInfo assemblyInfo;
    assemblyInfo->primitiveRestartEnable = VK_FALSE;
    assemblyInfo->topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    return assemblyInfo;
}

krt::PrimitiveAssemblyInfo::PrimitiveAssemblyInfo()
{
    m_VkAssemblyInfo = {};
    m_VkAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
}

krt::RasterizationStateInfo krt::RasterizationStateInfo::CreateDefault()
{
    RasterizationStateInfo rasterInfo;


    rasterInfo->depthClampEnable = VK_FALSE;
    rasterInfo->rasterizerDiscardEnable = VK_FALSE;
    rasterInfo->polygonMode = VK_POLYGON_MODE_FILL;
    rasterInfo->lineWidth = 1.0f;
    rasterInfo->cullMode = VK_CULL_MODE_BACK_BIT;
    rasterInfo->frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterInfo->depthBiasEnable = VK_FALSE;
    rasterInfo->depthBiasClamp = 0.0f;
    rasterInfo->depthBiasConstantFactor = 0.0f;
    rasterInfo->depthBiasSlopeFactor = 0.0f;

    return rasterInfo;
}

krt::RasterizationStateInfo::RasterizationStateInfo()
{
    m_VkRasterizationInfo = {};
    m_VkRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
}

krt::MultisampleInfo krt::MultisampleInfo::CreateDefault()
{
    MultisampleInfo  multisampleInfo;

    multisampleInfo->sampleShadingEnable = VK_FALSE;
    multisampleInfo->rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleInfo->minSampleShading = 1.0f;
    multisampleInfo->pSampleMask = nullptr;
    multisampleInfo->alphaToCoverageEnable = VK_FALSE;
    multisampleInfo->alphaToOneEnable = VK_FALSE;

    return multisampleInfo;
}

krt::MultisampleInfo::MultisampleInfo()
{
    m_VkMultisampleInfo = {};
    m_VkMultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
}

krt::ColorBlendAttachment krt::ColorBlendAttachment::CreateDefault()
{
    ColorBlendAttachment colorBlendAttachment;

    colorBlendAttachment->colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | 
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment->blendEnable = false;
    colorBlendAttachment->srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment->srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment->dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment->colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment->alphaBlendOp = VK_BLEND_OP_ADD;

    return colorBlendAttachment;
}

krt::ColorBlendAttachment::ColorBlendAttachment()
{
    m_VkColorBlendAttachment = {};
}

krt::ColorBlendInfo krt::ColorBlendInfo::CreateDefault()
{
    ColorBlendInfo colorBlendInfo;

    colorBlendInfo->logicOpEnable = VK_FALSE;
    colorBlendInfo->logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo->blendConstants[0] = 0.0f;
    colorBlendInfo->blendConstants[1] = 0.0f;
    colorBlendInfo->blendConstants[2] = 0.0f;
    colorBlendInfo->blendConstants[3] = 0.0f;

    return colorBlendInfo;
}

krt::ColorBlendInfo::ColorBlendInfo()
{
    m_VkColorBlendInfo = {};
    m_VkColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
}

krt::DepthStencilInfo krt::DepthStencilInfo::CreateDefault()
{
    DepthStencilInfo info;

    info->minDepthBounds = 0.0f;
    info->maxDepthBounds = 1.0f;
    info->depthTestEnable = VK_TRUE;
    info->depthWriteEnable = VK_TRUE;
    info->depthCompareOp = VK_COMPARE_OP_LESS;
    info->depthBoundsTestEnable = VK_TRUE;
    info->stencilTestEnable = VK_FALSE;

    return info;
}

krt::DepthStencilInfo::DepthStencilInfo()
{
    m_VkDepthStencilInfo = {};
    m_VkDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
}

void krt::PipelineLayoutInfo::AddLayoutBinding(uint32_t a_Set, uint32_t a_Binding, VkShaderStageFlags a_ShaderStage, VkDescriptorType a_Type, uint32_t a_Count)
{
    auto& binding = m_LayoutBindings[a_Set].emplace_back();
    binding.binding = a_Binding;
    binding.descriptorCount = a_Count;
    binding.descriptorType = a_Type;
    binding.stageFlags = a_ShaderStage;
    binding.pImmutableSamplers = nullptr;

}

std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& krt::PipelineLayoutInfo::GetDescriptorSetLayouts()
{
    return m_LayoutBindings;
}

krt::GraphicsPipeline::CreateInfo::CreateInfo()
    : m_VertexShaderFilepath("")
    , m_FragmentShaderFilepath("")
    , m_VertexInput()
    , m_PrimitiveAssemblyInfo(PrimitiveAssemblyInfo::CreateDefault())
    , m_RasterizationStateInfo(RasterizationStateInfo::CreateDefault())
    , m_MultisampleInfo(MultisampleInfo::CreateDefault())
    , m_ColorBlendAttachment(ColorBlendAttachment::CreateDefault())
    , m_ColorBlendInfo(ColorBlendInfo::CreateDefault())
    , m_DepthStencilInfo(DepthStencilInfo::CreateDefault())
    , m_DynamicStates(std::vector<VkDynamicState>())
    , m_Viewports(std::vector<VkViewport>())
    , m_ScissorRects(std::vector<VkRect2D>())
    , m_RenderPass(nullptr)
    , m_SubpassIndex(0)
{
}

krt::GraphicsPipeline::GraphicsPipeline(ServiceLocator& a_Services, CreateInfo& a_CreateInfo)
    : m_Services(a_Services)
{
    auto vertexModule = CreateShaderModule(a_CreateInfo.m_VertexShaderFilepath);
    auto fragmentModule = CreateShaderModule(a_CreateInfo.m_FragmentShaderFilepath);

    VkPipelineShaderStageCreateInfo stage = {};
    stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage.pName = "main";
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    assert(vertexModule != VK_NULL_HANDLE);
    stage.module = vertexModule;
    stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages.push_back(stage);

    if (fragmentModule != VK_NULL_HANDLE)
    {
        stage.module = fragmentModule;
        stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages.push_back(stage);
    }

    VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
    dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(a_CreateInfo.m_DynamicStates.size());
    dynamicStateInfo.pDynamicStates = a_CreateInfo.m_DynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.scissorCount = static_cast<uint32_t>(a_CreateInfo.m_ScissorRects.size());
    viewportInfo.pScissors = a_CreateInfo.m_ScissorRects.data();
    viewportInfo.viewportCount = static_cast<uint32_t>(a_CreateInfo.m_Viewports.size());
    viewportInfo.pViewports = a_CreateInfo.m_Viewports.data();

    CreateDescriptorSetPools(a_CreateInfo.m_PipelineLayout.GetDescriptorSetLayouts());
    auto descriptorSets = GetDescriptorLayouts();

    VkPipelineLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layoutInfo.pPushConstantRanges = nullptr;
    layoutInfo.pushConstantRangeCount = 0;
    layoutInfo.pSetLayouts = descriptorSets.data();
    layoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSets.size());

    ThrowIfFailed(vkCreatePipelineLayout(m_Services.m_LogicalDevice->GetVkDevice(), &layoutInfo, m_Services.m_AllocationCallbacks, &m_VkPipelineLayout));

    auto vertexInputInfo = a_CreateInfo.m_VertexInput.GetVkPipelineVertexInputStateCreateInfo();

    VkGraphicsPipelineCreateInfo graphicsPipelineInfo = {};
    graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineInfo.basePipelineIndex = -1;

    graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
    graphicsPipelineInfo.pInputAssemblyState = &a_CreateInfo.m_PrimitiveAssemblyInfo;
    graphicsPipelineInfo.pRasterizationState = &a_CreateInfo.m_RasterizationStateInfo;
    graphicsPipelineInfo.pMultisampleState = &a_CreateInfo.m_MultisampleInfo;
    graphicsPipelineInfo.pColorBlendState = &a_CreateInfo.m_ColorBlendInfo;
    graphicsPipelineInfo.pDepthStencilState = &a_CreateInfo.m_DepthStencilInfo;

    graphicsPipelineInfo.layout = m_VkPipelineLayout;
    graphicsPipelineInfo.pDynamicState = &dynamicStateInfo;
    graphicsPipelineInfo.pViewportState = &viewportInfo;

    graphicsPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    graphicsPipelineInfo.pStages = shaderStages.data();

    graphicsPipelineInfo.renderPass = a_CreateInfo.m_RenderPass->GetVkRenderPass();
    graphicsPipelineInfo.subpass = a_CreateInfo.m_SubpassIndex;

    ThrowIfFailed(vkCreateGraphicsPipelines(m_Services.m_LogicalDevice->GetVkDevice(), VK_NULL_HANDLE, 1,
        &graphicsPipelineInfo, m_Services.m_AllocationCallbacks, &m_VkPipeline));

    vkDestroyShaderModule(m_Services.m_LogicalDevice->GetVkDevice(), vertexModule, m_Services.m_AllocationCallbacks);
    vkDestroyShaderModule(m_Services.m_LogicalDevice->GetVkDevice(), fragmentModule, m_Services.m_AllocationCallbacks);
}

krt::GraphicsPipeline::~GraphicsPipeline()
{
    vkDestroyPipeline(m_Services.m_LogicalDevice->GetVkDevice(), m_VkPipeline, m_Services.m_AllocationCallbacks);
    vkDestroyPipelineLayout(m_Services.m_LogicalDevice->GetVkDevice(), m_VkPipelineLayout, m_Services.m_AllocationCallbacks);
}

std::unique_ptr<krt::DescriptorSetAllocation> krt::GraphicsPipeline::AllocateDescriptorSet(uint32_t a_Slot)
{
    return m_DescriptorSetPools[a_Slot]->GetDescriptorSet();
}

VkShaderModule krt::GraphicsPipeline::CreateShaderModule(std::string a_Filepath)
{
    if (!a_Filepath.empty())
    {
        auto bytecode = hlp::LoadFile(a_Filepath);

        auto shaderModule = hlp::CreateShaderModule(m_Services.m_LogicalDevice->GetVkDevice(), bytecode);

        return shaderModule;
    }


    return VK_NULL_HANDLE;
}

void krt::GraphicsPipeline::CreateDescriptorSetPools(
    std::map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>>& a_SetBindings)
{
    for (auto& set : a_SetBindings)
    {
        std::map<VkDescriptorType, uint32_t> descriptorMap;
        // Extract what types of descriptors are needed and in what amounts from the bindings.
        for (auto& binding : set.second)
        {
            descriptorMap[binding.descriptorType] += binding.descriptorCount;
        }

        // Yes, a magic number. TODO: Replace magic number with a value which can be specified during initialization of the pipeline
        auto pool = std::make_unique<DescriptorSetPool>(m_Services, set.second, descriptorMap, 32);

        m_DescriptorSetPools.emplace(std::make_pair(set.first, std::move(pool)));
    }
}

std::vector<VkDescriptorSetLayout> krt::GraphicsPipeline::GetDescriptorLayouts()
{
    std::vector<VkDescriptorSetLayout> layouts;
    for (auto& set : m_DescriptorSetPools)
    {
        layouts.push_back(set.second->GetSetLayout());
    }

    return layouts;
}
