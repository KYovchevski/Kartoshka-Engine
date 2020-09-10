#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "SemaphoreWait.h"

namespace krt
{
    struct ServiceLocator;
    class DescriptorSetAllocation;
    class GraphicsPipeline;
    class Sampler;
    class Texture;
    class Buffer;
    enum ECommandQueueType : uint8_t;
}

namespace krt
{
    class DescriptorSet
    {
    public:
        DescriptorSet(ServiceLocator& a_Services, GraphicsPipeline& a_Pipeline, uint32_t a_SetSlot, std::set<ECommandQueueType>& a_QueuesWithAccess);
        ~DescriptorSet();

        template<typename BufferStruct>
        void SetUniformBuffer(const BufferStruct& a_BufferStruct, uint32_t a_Binding,
            VkPipelineStageFlags a_UsingStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        void SetUniformBuffer(const void* a_Data, uint32_t a_DataSize, uint32_t a_Binding,
            VkPipelineStageFlags a_UsingStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

        template<typename BufferStruct>
        void SetStorageBuffer(const std::vector<BufferStruct>& a_StructList, uint32_t a_Binding,
            VkPipelineStageFlags a_UsingStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        void SetStorageBuffer(const void* a_Data, uint32_t a_DataSize, uint32_t a_Binding,
            VkPipelineStageFlags a_UsingStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

        void SetSampler(const Sampler& a_Sampler, uint32_t a_Binding);
        void SetTexture(const Texture& a_Texture, uint32_t a_Binding);

        GraphicsPipeline* GetGraphicsPipeline() const { return m_GraphicsPipeline; };

        VkDescriptorSet operator*() const;

        const std::vector<SemaphoreWait>& GetWaitSemaphores() const { return m_SemaphoreWaits; }
        void ClearWaitSemaphores() const { m_SemaphoreWaits.clear(); }

    private:

        krt::Semaphore CreateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding);
        krt::Semaphore UpdateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding);

        ServiceLocator& m_Services;

        // The handle to the allocated descriptor set in one of the descriptor set pools
        std::unique_ptr<DescriptorSetAllocation> m_DescriptorSetAllocation;

        // The graphics pipeline the descriptor set is associated with
        GraphicsPipeline* m_GraphicsPipeline;
        // The index of the set in the pipeline
        uint32_t m_SetSlot;

        struct DescriptorUpdate
        {
            VkDescriptorBufferInfo m_BufferUpdate;
            VkDescriptorImageInfo m_ImageUpdate;
            uint32_t m_TargetBinding;
            VkDescriptorType m_DescriptorType;
        };

        std::vector<DescriptorUpdate>  m_PendingDescriptorUpdates;

        std::set<ECommandQueueType> m_QueuesWithAccess;
        // Buffers created by the descriptor set dynamically
        std::map<uint32_t, std::unique_ptr<Buffer>> m_Buffers;

        mutable std::vector<SemaphoreWait> m_SemaphoreWaits;

        std::vector<Buffer> m_StagingBuffers;
    };

    
}

#include "DescriptorSet.inl"