#pragma once


#include "VkHelpers.h"

#include "vulkan/vulkan.h"

#include <glm/vec2.hpp>

#include <memory>
#include <set>
#include <vector>
#include <map>


namespace krt
{
    class GraphicsPipeline;
    struct ServiceLocator;
    class CommandQueue;
    class VertexBuffer;
    class RenderPass;
    class DescriptorSetAllocation;
    class Texture;
    class Sampler;

    enum ECommandQueueType : uint8_t;
}


namespace krt
{


    class CommandBuffer
    {
    public:
        CommandBuffer(ServiceLocator& a_Services, CommandQueue& a_CommandQueue);
        ~CommandBuffer();

        CommandBuffer(CommandBuffer&) = delete;             // No copy c-tor
        CommandBuffer(CommandBuffer&&) = delete;            // No move c-tor
        CommandBuffer& operator=(CommandBuffer&) = delete;  // No copy assignment
        CommandBuffer& operator=(CommandBuffer&&) = delete; // No move assignment

        void Reset();
        void Begin();
        void End();
        void Submit();

        void AddWaitSemaphore(VkSemaphore a_Semaphore);
        void AddSignalSemaphore(VkSemaphore a_Semaphore);
        void SetWaitStages(VkPipelineStageFlags a_WaitStages);

        void BeginRenderPass(RenderPass& a_RenderPass, VkFramebuffer a_FrameBuffer, VkRect2D a_RenderArea, VkSubpassContents a_SubpassContents = VK_SUBPASS_CONTENTS_INLINE);
        void EndRenderPass();

        void SetViewport(const VkViewport& a_Viewport);
        void SetScissorRect(const VkRect2D& a_Scissor);

        void SetVertexBuffer(VertexBuffer& a_VertexBuffer, uint32_t a_Binding, VkDeviceSize a_Offset = 0);
        void BindPipeline(GraphicsPipeline& a_Pipeline);

        void Draw(uint32_t a_NumVertices, uint32_t a_NumInstances = 1, uint32_t a_FirstVertex = 0, uint32_t a_FirstInstance = 0);
        void DrawIndexed(uint32_t a_NumIndices, uint32_t a_NumInstances = 1, uint32_t a_FirstIndex = 0, uint32_t a_FirstInstance = 0, uint32_t a_VertexOffset = 0);

        const std::set<VkSemaphore>& GetWaitSemaphores() const { return m_WaitSemaphores; }
        const std::set<VkSemaphore>& GetSignalSemaphores() const { return m_SignalSemaphores; };
        const VkPipelineStageFlags GetWaitStagesMask() const { return m_WaitStages; }

        std::unique_ptr<Texture> CreateTextureFromFile(std::string a_Filepath, std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        std::unique_ptr<Texture> CreateTexture(void* a_Data, glm::uvec2 a_Dimensions, const uint8_t a_NumChannels, const uint8_t a_BytesPerChannel,
            std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        template<typename DataType>
        void SetUniformBuffer(const DataType& a_Data, uint32_t a_Binding, uint32_t a_Set);

        void SetSampler(Sampler& a_Sampler, uint32_t a_Binding, uint32_t a_Set);
        void SetTexture(Texture& a_Texture, uint32_t a_Binding, uint32_t a_Set);

        template<typename AttributeType>
        std::unique_ptr<VertexBuffer> CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess);

        VkCommandBuffer GetVkCommandBuffer();


    private:

        void BindDescriptorSets();

        void TransitionImageLayout(VkImage a_VkImage, VkImageLayout a_OldLayout, VkImageLayout a_NewLayout, VkAccessFlags a_SrcAccessMask, VkAccessFlags
                                   a_DstAccessMask, VkPipelineStageFlags a_SrcStageMask, VkPipelineStageFlags a_DstStageMask);

        std::unique_ptr<VertexBuffer> CreateVertexBuffer(void* a_BufferData, uint64_t a_NumElements,
            uint64_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess);

        void SetUniformBuffer(const void* a_Data, uint64_t a_DataSize, uint32_t a_Binding, uint32_t a_Set);

        std::pair<VkBuffer, VkDeviceMemory> CreateBufferElements(uint64_t a_Size, VkBufferUsageFlags a_Usage, VkMemoryPropertyFlags a_MemoryProperties,
            const std::set<ECommandQueueType>& a_QueuesWithAccess);

        void CopyToDeviceMemory(VkDeviceMemory a_DeviceMemory, const void* a_Data, VkDeviceSize a_Size);

        std::vector<uint32_t> GetQueueIndices(const std::set<ECommandQueueType>& a_Queues);

        ServiceLocator& m_Services;

        VkCommandBuffer m_VkCommandBuffer;

        CommandQueue& m_CommandQueue;

        std::vector<VkBuffer> m_IntermediateBuffers;
        std::vector<VkDeviceMemory> m_IntermediateBufferMemoryAllocations;
        std::vector<std::unique_ptr<DescriptorSetAllocation>> m_IntermediateDescriptorSetAllocations;

        std::set<VkSemaphore> m_WaitSemaphores;
        std::set<VkSemaphore> m_SignalSemaphores;
        VkPipelineStageFlags m_WaitStages;

        GraphicsPipeline* m_CurrentGraphicsPipeline;

        struct DescriptorUpdate
        {
            VkDescriptorBufferInfo m_BufferUpdate;
            VkDescriptorImageInfo m_ImageUpdate;
            uint32_t m_TargetBinding;
            VkDescriptorType m_DescriptorType;
        };

        std::map<uint32_t, std::vector<DescriptorUpdate>>  m_PendingDescriptorUpdates;

        bool m_HasBegun;
    };

#include "CommandBuffer.inl"
}
