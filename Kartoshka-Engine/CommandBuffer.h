#pragma once


#include "VkHelpers.h"

#include "vulkan/vulkan.h"

#include <glm/vec2.hpp>

#include <memory>
#include <set>
#include <vector>
#include <map>

#include "Framebuffer.h"
#include "SemaphoreAllocator.h"
#include "SemaphoreWait.h"


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
    class Buffer;
    class DescriptorSet;
    class IndexBuffer;
    class Material;

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

        // Adds a semaphore that should be signaled once the command buffer's execution is finished
        void AddSignalSemaphore(Semaphore a_Semaphore);
        // Adds a semaphore that should be signaled before the execution of the command buffer begins 
        void AddWaitSemaphore(Semaphore a_Semaphore, VkPipelineStageFlags a_StageFlags);

        void BeginRenderPass(RenderPass& a_RenderPass, Framebuffer& a_FrameBuffer, VkRect2D a_RenderArea, VkSubpassContents a_SubpassContents = VK_SUBPASS_CONTENTS_INLINE);
        void EndRenderPass();

        void SetViewport(const VkViewport& a_Viewport);
        void SetScissorRect(const VkRect2D& a_Scissor);

        void SetVertexBuffer(VertexBuffer& a_VertexBuffer, uint32_t a_Binding, VkDeviceSize a_Offset = 0);
        void SetIndexBuffer(IndexBuffer& a_IndexBuffer, uint32_t a_Offset = 0);
        void BindPipeline(GraphicsPipeline& a_Pipeline);
        
        void SetDescriptorSet(DescriptorSet& a_Set, uint32_t a_Slot);
        void SetSampler(Sampler& a_Sampler, uint32_t a_Binding, uint32_t a_Set);
        void SetTexture(Texture& a_Texture, uint32_t a_Binding, uint32_t a_Set);

        void SetMaterial(Material& a_Material, uint32_t a_Set);

        void Draw(uint32_t a_NumVertices, uint32_t a_NumInstances = 1, uint32_t a_FirstVertex = 0, uint32_t a_FirstInstance = 0);
        void DrawIndexed(uint32_t a_NumIndices, uint32_t a_NumInstances = 1, uint32_t a_FirstIndex = 0, uint32_t a_FirstInstance = 0, uint32_t a_VertexOffset = 0);

        const std::set<Semaphore>& GetSignalSemaphores() const { return m_SignalSemaphores; }
        const std::vector<SemaphoreWait>& GetWaitSemaphores() const;

        std::unique_ptr<Texture> CreateTextureFromFile(std::string a_Filepath, std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        std::unique_ptr<Texture> CreateTexture(void* a_Data, glm::uvec2 a_Dimensions, const uint8_t a_NumChannels, const uint8_t a_BytesPerChannel,
            std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        template<typename DataType>
        void SetUniformBuffer(const DataType& a_Data, uint32_t a_Binding, uint32_t a_Set);

        template<typename AttributeType>
        std::unique_ptr<VertexBuffer> CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess);
        std::unique_ptr<VertexBuffer> CreateVertexBuffer(void* a_BufferData, uint64_t a_NumElements,
            uint64_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess);

        template<typename IndexType>
        std::unique_ptr<IndexBuffer> CreateIndexBuffer(std::vector<IndexType> a_Indices, std::set<ECommandQueueType> a_QueuesWithAccess);
        std::unique_ptr<IndexBuffer> CreateIndexBuffer(void* a_IndexData, uint64_t a_NumElements,
            uint8_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess);

        template<typename UniformType>
        void PushConstant(const UniformType& a_Uniform, uint32_t a_Slot);


        VkCommandBuffer GetVkCommandBuffer();

        // Performs from the source Buffer object to the destination Buffer object based on the given copy region size and offsets
        void BufferCopy(Buffer& a_SourceBuffer, Buffer& a_DestinationBuffer, VkDeviceSize a_Size, VkDeviceSize a_SourceOffset = 0, VkDeviceSize a_DestinationOffset = 0);
        // Performs from the source Buffer object to the destination Buffer object based on the given copy region size and offsets
        void BufferCopy(VkBuffer a_SourceBuffer, VkBuffer a_DestinationBuffer, VkDeviceSize a_Size, VkDeviceSize a_SourceOffset = 0, VkDeviceSize a_DestinationOffset = 0);

        // Transfers the CPU data to a GPU buffer, even if the buffer is not in host visible memory.
        // Returns true if the target buffer was resized, false otherwise.
        bool UploadToBuffer(const void* a_Data, VkDeviceSize a_DataSize, Buffer& a_TargetBuffer);
        void TransitionImageLayout(VkImage a_VkImage, VkImageLayout a_OldLayout, VkImageLayout a_NewLayout, VkAccessFlags a_SrcAccessMask, VkAccessFlags
                                   a_DstAccessMask, VkPipelineStageFlags a_SrcStageMask, VkPipelineStageFlags a_DstStageMask);
    private:

        void BindDescriptorSets();




        void SetUniformBuffer(const void* a_Data, uint64_t a_DataSize, uint32_t a_Binding, uint32_t a_Set);

        void PushConstant(const void* a_Data, uint32_t a_DataSize, uint32_t a_Slot);

        ServiceLocator& m_Services;

        VkCommandBuffer m_VkCommandBuffer;
        CommandQueue& m_CommandQueue;

        std::vector<std::unique_ptr<Buffer>> m_IntermediateBuffers;
        std::vector<std::unique_ptr<DescriptorSetAllocation>> m_IntermediateDescriptorSetAllocations;

        std::set<Semaphore> m_SignalSemaphores;
        mutable std::vector<SemaphoreWait> m_WaitSemaphores;
        std::vector<DescriptorSet*> m_InUseDescriptorSets;

        GraphicsPipeline* m_CurrentGraphicsPipeline;

        struct DescriptorUpdate
        {
            VkDescriptorBufferInfo m_BufferUpdate;
            VkDescriptorImageInfo m_ImageUpdate;
            uint32_t m_TargetBinding;
            VkDescriptorType m_DescriptorType;
        };

        std::map<uint32_t, std::vector<DescriptorUpdate>>  m_PendingDescriptorUpdates;
        std::map<uint32_t, VkDescriptorSet> m_CurrentlyBoundDescriptorSets;

        bool m_HasBegun;
    };

#include "CommandBuffer.inl"
}
