#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <memory>
#include <set>
#include <vector>

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
        void SetUniformBuffer(const BufferStruct& a_BufferStruct, uint32_t a_Binding);
        void SetUniformBuffer(const void* a_Data, uint32_t a_DataSize, uint32_t a_Binding);

        void SetSampler(const Sampler& a_Sampler, uint32_t a_Binding);
        void SetTexture(const Texture& a_Texture, uint32_t a_Binding);

        GraphicsPipeline* GetGraphicsPipeline() const { return m_GraphicsPipeline; };

        VkDescriptorSet operator*() const;

    private:

        void CreateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding);
        void UpdateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding);

        ServiceLocator& m_Services;

        std::unique_ptr<DescriptorSetAllocation> m_DescriptorSetAllocation;

        GraphicsPipeline* m_GraphicsPipeline;
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
        std::map<uint32_t, std::unique_ptr<Buffer>> m_Buffers;

    };

    
}

#include "DescriptorSet.inl"