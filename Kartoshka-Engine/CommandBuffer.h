#pragma once

#include "VkHelpers.h"

#include "vulkan/vulkan.h"

#include <memory>
#include <set>
#include <vector>


namespace krt
{
    class GraphicsPipeline;
    struct ServiceLocator;
    class CommandQueue;
    class VertexBuffer;
    class RenderPass;

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

        template<typename AttributeType>
        std::unique_ptr<VertexBuffer> CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess);

        VkCommandBuffer GetVkCommandBuffer();

    private:

        std::unique_ptr<VertexBuffer> CreateVertexBuffer(void* a_BufferData, uint64_t a_NumElements,
            uint64_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess);

        ServiceLocator& m_Services;

        VkCommandBuffer m_VkCommandBuffer;

        CommandQueue& m_CommandQueue;

        std::vector<VkBuffer> m_StagingBuffers;
        std::vector<VkDeviceMemory> m_StagingBufferMemoryAllocations;

        std::set<VkSemaphore> m_WaitSemaphores;
        std::set<VkSemaphore> m_SignalSemaphores;
        VkPipelineStageFlags m_WaitStages;

        bool m_HasBegun;
    };

#include "CommandBuffer.inl"
}
