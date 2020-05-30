#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace krt
{
    struct ServiceLocator;
    class RenderPass;
    class CommandBuffer;
}

namespace krt
{
    class VertexInputInfo
    {
    public:

        VkPipelineVertexInputStateCreateInfo GetVkPipelineVertexInputStateCreateInfo();

        template<typename AttributeType>
        void AddPerVertexAttribute(uint32_t a_Binding, uint32_t a_Location, VkFormat a_Format);

        template<typename AttributeType>
        void AddPerInstanceAttribute(uint32_t a_Binding, uint32_t a_Location, VkFormat a_Format);
    private:

        uint32_t FindOffset(uint32_t a_Binding);
        void CorrectStrides(uint32_t a_Binding, uint32_t a_NewStride);

        std::unordered_map<uint32_t, VkVertexInputBindingDescription> m_Bindings;
        std::vector<VkVertexInputAttributeDescription> m_Attributes;

    };

    class PrimitiveAssemblyInfo
    {
    public:
        VkPipelineInputAssemblyStateCreateInfo* operator->() { return &m_VkAssemblyInfo; }
        VkPipelineInputAssemblyStateCreateInfo* operator&() { return &m_VkAssemblyInfo; }
        static PrimitiveAssemblyInfo CreateDefault();

    private:
        PrimitiveAssemblyInfo();
        VkPipelineInputAssemblyStateCreateInfo m_VkAssemblyInfo;
    };

    class RasterizationStateInfo
    {
    public:
        VkPipelineRasterizationStateCreateInfo* operator->() { return &m_VkRasterizationInfo; }
        VkPipelineRasterizationStateCreateInfo* operator&() { return &m_VkRasterizationInfo; }
        static RasterizationStateInfo CreateDefault();

    private:
        RasterizationStateInfo();
        VkPipelineRasterizationStateCreateInfo m_VkRasterizationInfo;

    };

    class MultisampleInfo
    {
    public:
        VkPipelineMultisampleStateCreateInfo* operator->() { return &m_VkMultisampleInfo; }
        VkPipelineMultisampleStateCreateInfo* operator&() { return &m_VkMultisampleInfo; }
        static MultisampleInfo CreateDefault();

    private:
        MultisampleInfo();
        VkPipelineMultisampleStateCreateInfo m_VkMultisampleInfo;
    };

    class ColorBlendAttachment
    {
    public:
        VkPipelineColorBlendAttachmentState* operator->() { return &m_VkColorBlendAttachment; }
        VkPipelineColorBlendAttachmentState* operator&() { return &m_VkColorBlendAttachment; }
        static ColorBlendAttachment CreateDefault();

    private:
        ColorBlendAttachment();
        VkPipelineColorBlendAttachmentState m_VkColorBlendAttachment;
    };


    class ColorBlendInfo
    {
    public:
        VkPipelineColorBlendStateCreateInfo* operator->() { return &m_VkColorBlendInfo; }
        VkPipelineColorBlendStateCreateInfo* operator&() { return &m_VkColorBlendInfo; }

        static ColorBlendInfo CreateDefault();

    private:
        ColorBlendInfo();
        VkPipelineColorBlendStateCreateInfo m_VkColorBlendInfo;
    };

    class DepthStencilInfo
    {
    public:
        VkPipelineDepthStencilStateCreateInfo* operator->() { return &m_VkDepthStencilInfo; }
        VkPipelineDepthStencilStateCreateInfo* operator&() { return &m_VkDepthStencilInfo; }
        static DepthStencilInfo CreateDefault();

    private:
        DepthStencilInfo();
        VkPipelineDepthStencilStateCreateInfo m_VkDepthStencilInfo;
    };

    class GraphicsPipeline
    {
        friend CommandBuffer;
    public:
        struct CreateInfo
        {
            CreateInfo();

            std::string m_VertexShaderFilepath;
            std::string m_FragmentShaderFilepath;

            VertexInputInfo m_VertexInput;
            PrimitiveAssemblyInfo m_PrimitiveAssemblyInfo;
            RasterizationStateInfo m_RasterizationStateInfo;
            MultisampleInfo m_MultisampleInfo;
            ColorBlendAttachment m_ColorBlendAttachment;
            ColorBlendInfo m_ColorBlendInfo;

            std::vector<VkDynamicState> m_DynamicStates;
            std::vector<VkViewport> m_Viewports;
            std::vector<VkRect2D> m_ScissorRects;

            RenderPass* m_RenderPass;
            uint32_t m_SubpassIndex;
        };

        GraphicsPipeline(ServiceLocator& a_Services, CreateInfo& a_CreateInfo);
        ~GraphicsPipeline();
         
        VkPipeline GetVkPipeline() const { return m_VkPipeline; }

    private:
        VkShaderModule CreateShaderModule(std::string a_Filepath);

        ServiceLocator& m_Services;

        VkPipelineLayout m_VkPipelineLayout;
        VkPipeline m_VkPipeline;
    };

#include "GraphicsPipeline.inl"


}