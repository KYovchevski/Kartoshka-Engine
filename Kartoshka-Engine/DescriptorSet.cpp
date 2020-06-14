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

void krt::DescriptorSet::SetUniformBuffer(const void* a_Data, uint32_t a_DataSize, uint32_t a_Binding)
{
    // TODO: Add updating to the buffer if one already exists
    CreateUniformBuffer(a_Data, a_DataSize, a_Binding);
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


void krt::DescriptorSet::CreateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding)
{
    auto staging = m_Services.m_LogicalDevice->CreateBuffer(a_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_QueuesWithAccess);

    auto buffer = m_Services.m_LogicalDevice->CreateBuffer(a_Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_QueuesWithAccess);

    m_Services.m_LogicalDevice->CopyToDeviceMemory(staging->m_VkDeviceMemory, a_Data, a_Size);

    auto& transferQueue = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue);
    auto& transferBuffer = transferQueue.GetSingleUseCommandBuffer();

    transferBuffer.Begin();
    transferBuffer.BufferCopy(*staging, *buffer, a_Size);
    transferBuffer.Submit();

    m_Buffers[a_Binding] = std::move(buffer);

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

    // TODO: Replace this with something that isn't an immediate flush of the command queue
    // Maybe check if the command buffer has finished executing before binding?
    transferQueue.Flush();

    vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), 1, &write, 0, nullptr);

}

void krt::DescriptorSet::UpdateUniformBuffer(const void* a_Data, uint32_t a_Size, uint32_t a_Binding)
{
    a_Binding;
    //TODO: Implement this properly
    auto staging = m_Services.m_LogicalDevice->CreateBuffer(a_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_QueuesWithAccess);

    m_Services.m_LogicalDevice->CopyToDeviceMemory(staging->m_VkDeviceMemory, a_Data, a_Size);

}   
