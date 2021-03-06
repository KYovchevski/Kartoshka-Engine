#include "DescriptorSet.h"
#include "DescriptorSetAllocation.h"
#include "GraphicsPipeline.h"
#include "Buffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "Sampler.h"
#include "Texture.h"
#include "SemaphoreAllocator.h"

krt::DescriptorSet::DescriptorSet(ServiceLocator& a_Services, GraphicsPipeline& a_Pipeline, uint32_t a_SetSlot,
    std::set<ECommandQueueType>& a_QueuesWithAccess)
    : m_Services(a_Services)
    , m_GraphicsPipeline(&a_Pipeline)
    , m_SetSlot(a_SetSlot)
    , m_QueuesWithAccess(a_QueuesWithAccess)
{
    m_DescriptorSetAllocation = m_GraphicsPipeline->AllocateDescriptorSet(a_SetSlot);
}

krt::DescriptorSet::~DescriptorSet()
{
    if (m_DescriptorSetAllocation)
        m_DescriptorSetAllocation.reset();
}

krt::SemaphoreWait krt::DescriptorSet::SetUniformBuffer(const void* a_Data, VkDeviceSize a_DataSize, uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    SemaphoreWait& semWait = m_SemaphoreWaits.emplace_back();
    semWait.m_StageFlags = a_UsingStage;

    if (m_Buffers.count(a_Binding) == 0 || m_Buffers.at(a_Binding)->m_BufferSize < a_DataSize)
    {
        // No buffer for this binding exists already, or the buffer is too small for the data
        semWait.m_Semaphore = CreateBuffer(a_Data, a_DataSize, a_Binding, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

        // Since a new buffer was created and stored in m_Buffers, the descriptor set needs to be updated
        // to use that new buffer
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        bufferInfo.buffer = m_Buffers[a_Binding]->m_VkBuffer;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.dstArrayElement = 0;
        write.dstBinding = a_Binding;
        write.dstSet = **m_DescriptorSetAllocation;
        write.pBufferInfo = &bufferInfo;

        // The descriptor set update does not need to wait for the transfer commands to be executed, so we don't need to synchronize immediately.
        vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), 1, &write, 0, nullptr);
    }
    else
    {
        // A buffer for this binding already exists and is bound to the descriptor set
        // All that needs to be done is update the data in it
        semWait.m_Semaphore = UpdateBuffer(a_Data, a_DataSize, a_Binding);
    }

    return semWait;
}

krt::SemaphoreWait krt::DescriptorSet::SetStorageBuffer(const void* a_Data, VkDeviceSize a_DataSize, uint32_t a_Binding,
                                                        VkPipelineStageFlags a_UsingStage)
{
    SemaphoreWait semWait;
    semWait.m_StageFlags = a_UsingStage;

    if (m_Buffers.count(a_Binding) == 0 || m_Buffers.at(a_Binding)->m_BufferSize < a_DataSize)
    {
        semWait.m_Semaphore = CreateBuffer(a_Data, a_DataSize, a_Binding, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_Buffers[a_Binding]->m_VkBuffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstBinding = a_Binding;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write.dstSet = **m_DescriptorSetAllocation;
        write.dstArrayElement = 0;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), 1, &write, 0, nullptr);
    }
    else
    {
        semWait.m_Semaphore = UpdateBuffer(a_Data, a_DataSize, a_Binding);
    }

    return semWait;
}


void krt::DescriptorSet::SetSampler(const Sampler& a_Sampler, uint32_t a_Binding)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.sampler = *a_Sampler;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    write.descriptorCount = 1;
    write.dstArrayElement = 0;
    write.dstBinding = a_Binding;
    write.dstSet = **m_DescriptorSetAllocation;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), 1, &write, 0, nullptr);

}

void krt::DescriptorSet::SetTexture(const Texture& a_Texture, uint32_t a_Binding)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = a_Texture.GetVkImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = 1;
    write.dstArrayElement = 0;
    write.dstBinding = a_Binding;
    write.dstSet = **m_DescriptorSetAllocation;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), 1, &write, 0, nullptr);
}

VkDescriptorSet krt::DescriptorSet::operator*() const
{
    return **m_DescriptorSetAllocation;
}


krt::Semaphore krt::DescriptorSet::CreateBuffer(const void* a_Data, VkDeviceSize a_Size, uint32_t a_Binding, VkBufferUsageFlags a_UsageFlags)
{
    auto buffer = m_Services.m_LogicalDevice->CreateBuffer(a_Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | a_UsageFlags,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_QueuesWithAccess);

    auto& transferQueue = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue);
    auto& transferBuffer = transferQueue.GetSingleUseCommandBuffer();

    // The state of the command buffer needs to be tracked to ensure it has been executed before the descriptor set is used
    auto sem = m_Services.m_SemaphoreAllocator->GetSemaphore();

    transferBuffer.Begin();
    // Upload to the permanent buffer
    transferBuffer.UploadToBuffer(a_Data, a_Size, *buffer);
    transferBuffer.AddSignalSemaphore(sem);
    transferBuffer.End();
    transferBuffer.Submit();

    // The descriptor set is the owner of the buffer, so it is the holder of the unique_ptr
    m_Buffers[a_Binding] = std::move(buffer);

    return sem;
}

krt::Semaphore krt::DescriptorSet::UpdateBuffer(const void* a_Data, VkDeviceSize a_Size, uint32_t a_Binding)
{
    a_Binding;
    //TODO: Implement this properly

    auto& buffer = m_Buffers[a_Binding];

    // Need to use a command buffer to transfer the data from host visible memory to the more optimized device local memory
    auto& queue = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue);
    auto& commandBuffer = queue.GetSingleUseCommandBuffer();

    // The command buffer will signal a semaphore so that command buffers dependent on this descriptor set don't use it before it is ready
    auto sem = m_Services.m_SemaphoreAllocator->GetSemaphore();

    commandBuffer.Begin();
    commandBuffer.UploadToBuffer(a_Data, a_Size, *buffer);
    commandBuffer.AddSignalSemaphore(sem);
    commandBuffer.End();
    commandBuffer.Submit();

    return sem;
}   
