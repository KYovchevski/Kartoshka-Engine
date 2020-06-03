﻿#include "CommandBuffer.h"

#include "ServiceLocator.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandQueue.h"
#include "VertexBuffer.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"
#include "DescriptorSetPool.h"
#include "DescriptorSetAllocation.h"
#include "Texture.h"
#include "Sampler.h"

#include "stb/stb_image.h"

#include "VkHelpers.h"

krt::CommandBuffer::CommandBuffer(ServiceLocator& a_Services, CommandQueue& a_CommandQueue)
    : m_Services(a_Services)
    , m_CommandQueue(a_CommandQueue)
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandQueue.GetVkCommandPool();
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, &m_VkCommandBuffer);
}

krt::CommandBuffer::~CommandBuffer()
{
    vkFreeCommandBuffers(m_Services.m_LogicalDevice->GetVkDevice(), m_CommandQueue.GetVkCommandPool(), 1, &m_VkCommandBuffer);
}

void krt::CommandBuffer::Reset()
{
    m_WaitSemaphores.clear();
    m_SignalSemaphores.clear();

    for (auto memory : m_IntermediateBufferMemoryAllocations)
        vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), memory, m_Services.m_AllocationCallbacks);

    m_IntermediateBufferMemoryAllocations.clear();

    for (auto stagingBuffer : m_IntermediateBuffers)
        vkDestroyBuffer(m_Services.m_LogicalDevice->GetVkDevice(), stagingBuffer, m_Services.m_AllocationCallbacks);

    m_IntermediateBuffers.clear();

    for (auto& descriptorSetAllocation : m_IntermediateDescriptorSetAllocations)
        descriptorSetAllocation.reset();

    m_IntermediateDescriptorSetAllocations.clear();

    vkResetCommandBuffer(m_VkCommandBuffer, 0);
}

void krt::CommandBuffer::Begin()
{
    if (!m_HasBegun)
    {
        m_HasBegun = true;
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(m_VkCommandBuffer, &info);
    }
}

void krt::CommandBuffer::End()
{
    if (m_HasBegun)
    {
        m_HasBegun = false;
        vkEndCommandBuffer(m_VkCommandBuffer);
    }
}

void krt::CommandBuffer::Submit()
{
    CommandBuffer& commandBuffer = *this;
    m_CommandQueue.SubmitCommandBuffer(commandBuffer);
}

void krt::CommandBuffer::AddWaitSemaphore(VkSemaphore a_Semaphore)
{
    m_WaitSemaphores.insert(a_Semaphore);
}

void krt::CommandBuffer::AddSignalSemaphore(VkSemaphore a_Semaphore)
{
    m_SignalSemaphores.insert(a_Semaphore);
}

void krt::CommandBuffer::SetWaitStages(VkPipelineStageFlags a_WaitStages)
{
    m_WaitStages = a_WaitStages;
}

void krt::CommandBuffer::BeginRenderPass(RenderPass& a_RenderPass, VkFramebuffer a_FrameBuffer, VkRect2D a_RenderArea, VkSubpassContents a_SubpassContents)
{
    // TODO: the clear value is hardcoded, this should most likely be part of a custom Framebuffer class so that the clear value can be specified for all images
    VkClearValue clearValue = { 0.4f, 0.5f, 0.9f,1.0f };

    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.framebuffer = a_FrameBuffer;
    info.renderPass = a_RenderPass.GetVkRenderPass();
    info.clearValueCount = 1;
    info.pClearValues = &clearValue;
    info.renderArea = a_RenderArea;
    vkCmdBeginRenderPass(m_VkCommandBuffer, &info, a_SubpassContents);
}

void krt::CommandBuffer::EndRenderPass()
{
    vkCmdEndRenderPass(m_VkCommandBuffer);
}

void krt::CommandBuffer::SetViewport(const VkViewport& a_Viewport)
{
    vkCmdSetViewport(m_VkCommandBuffer, 0, 1, &a_Viewport);
}

void krt::CommandBuffer::SetScissorRect(const VkRect2D& a_Scissor)
{
    vkCmdSetScissor(m_VkCommandBuffer, 0, 1, &a_Scissor);
}

void krt::CommandBuffer::SetVertexBuffer(VertexBuffer& a_VertexBuffer, uint32_t a_Binding, VkDeviceSize a_Offset)
{
    vkCmdBindVertexBuffers(m_VkCommandBuffer, a_Binding, 1, &a_VertexBuffer.m_VkBuffer, &a_Offset);
}

void krt::CommandBuffer::BindPipeline(GraphicsPipeline& a_Pipeline)
{
    vkCmdBindPipeline(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, a_Pipeline.m_VkPipeline);
    m_CurrentGraphicsPipeline = &a_Pipeline;
}

void krt::CommandBuffer::Draw(uint32_t a_NumVertices, uint32_t a_NumInstances, uint32_t a_FirstVertex,
    uint32_t a_FirstInstance)
{
    BindDescriptorSets();
    vkCmdDraw(m_VkCommandBuffer, a_NumVertices, a_NumInstances, a_FirstVertex, a_FirstInstance);
}

void krt::CommandBuffer::DrawIndexed(uint32_t a_NumIndices, uint32_t a_NumInstances, uint32_t a_FirstIndex,
    uint32_t a_FirstInstance, uint32_t a_VertexOffset)
{
    BindDescriptorSets();
    vkCmdDrawIndexed(m_VkCommandBuffer, a_NumIndices, a_NumInstances, a_FirstIndex, a_VertexOffset, a_FirstInstance);
}


std::unique_ptr<krt::Texture> krt::CommandBuffer::CreateTextureFromFile(std::string a_Filepath,
    std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages)
{
    int width, height, channels;
    auto data = stbi_load(a_Filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    glm::uvec2 dimensions(static_cast<uint32_t>(width), static_cast<uint32_t>(height));

    auto tex = CreateTexture(data, dimensions, 4, 1, a_QueuesWithAccess, a_UsingStages);

    stbi_image_free(data);

    return tex;
}

std::unique_ptr<krt::Texture> krt::CommandBuffer::CreateTexture(void* a_Data, glm::uvec2 a_Dimensions,
         const uint8_t a_NumChannels, const uint8_t a_BytesPerChannel, std::set<ECommandQueueType> a_QueuesWithAccess, VkPipelineStageFlags a_UsingStages)
{
    assert(a_NumChannels <= 4);
    assert(a_BytesPerChannel <= 8);

    VkFormat imageFormat = hlp::PickTextureFormat(a_NumChannels);

    uint64_t sizeInBytes = a_Dimensions.x * a_Dimensions.y * a_NumChannels * a_BytesPerChannel;
   
    std::unique_ptr<Texture> texture = std::make_unique<Texture>(m_Services);

    VkExtent3D extent;
    extent.width = a_Dimensions.x;
    extent.height = a_Dimensions.y;
    extent.depth = 1;

    auto queueIndices = GetQueueIndices(a_QueuesWithAccess);

    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = extent;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.pQueueFamilyIndices = queueIndices.data();
    imageInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueIndices.size());
    imageInfo.sharingMode = queueIndices.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    ThrowIfFailed(vkCreateImage(m_Services.m_LogicalDevice->GetVkDevice(), &imageInfo, m_Services.m_AllocationCallbacks, &texture->m_VkImage));

    auto memInfo = m_Services.m_PhysicalDevice->GetMemoryInfoForImage(texture->m_VkImage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    auto stagingBuffer = CreateBufferElements(memInfo.m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, { ETransferQueue });

    CopyToDeviceMemory(stagingBuffer.second, a_Data, sizeInBytes);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memInfo.m_Size;
    allocInfo.memoryTypeIndex = memInfo.m_MemoryType;

    ThrowIfFailed(vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, m_Services.m_AllocationCallbacks, &texture->m_VkDeviceMemory));
    ThrowIfFailed(vkBindImageMemory(m_Services.m_LogicalDevice->GetVkDevice(), texture->m_VkImage, texture->m_VkDeviceMemory, 0));

    TransitionImageLayout(texture->m_VkImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkBufferImageCopy copy;
    copy.bufferImageHeight = 0;
    copy.bufferOffset = 0;
    copy.bufferRowLength = 0;
    

    copy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy.imageSubresource.baseArrayLayer = 0;
    copy.imageSubresource.layerCount = 1;
    copy.imageSubresource.mipLevel = 0;

    copy.imageOffset = { 0,0,0 };
    copy.imageExtent = { a_Dimensions.x, a_Dimensions.y, 1 };

    vkCmdCopyBufferToImage(m_VkCommandBuffer, stagingBuffer.first, texture->m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    TransitionImageLayout(texture->m_VkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, a_UsingStages);

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.format = imageFormat;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.image = texture->m_VkImage;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;

    vkCreateImageView(m_Services.m_LogicalDevice->GetVkDevice(), &viewInfo, m_Services.m_AllocationCallbacks, &texture->m_VkImageView);

    m_IntermediateBuffers.push_back(stagingBuffer.first);
    m_IntermediateBufferMemoryAllocations.push_back(stagingBuffer.second);

    return texture;
}

void krt::CommandBuffer::SetSampler(Sampler& a_Sampler, uint32_t a_Binding, uint32_t a_Set)
{
   

    auto& update = m_PendingDescriptorUpdates[a_Set].emplace_back();
    update.m_ImageUpdate = {};
    update.m_TargetBinding = a_Binding;
    update.m_ImageUpdate.sampler = a_Sampler.m_VkSampler;
    update.m_DescriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
}

void krt::CommandBuffer::SetTexture(Texture& a_Texture, uint32_t a_Binding, uint32_t a_Set)
{
    auto& update = m_PendingDescriptorUpdates[a_Set].emplace_back();
    update.m_ImageUpdate = {};
    update.m_ImageUpdate.imageView = a_Texture.m_VkImageView;
    update.m_ImageUpdate.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    update.m_TargetBinding = a_Binding;
    update.m_DescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
}

VkCommandBuffer krt::CommandBuffer::GetVkCommandBuffer()
{
    return m_VkCommandBuffer;
}

void krt::CommandBuffer::BindDescriptorSets()
{
    for (auto& pending : m_PendingDescriptorUpdates)
    {
        auto descriptorAlloc = m_CurrentGraphicsPipeline->m_DescriptorSetPools[pending.first]->GetDescriptorSet();
        auto set = **descriptorAlloc;

        std::vector<VkWriteDescriptorSet> writes;

        for (auto& change : pending.second)
        {
            VkWriteDescriptorSet& writeInfo = writes.emplace_back();
            writeInfo = {};
            writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeInfo.dstSet = set;
            writeInfo.dstBinding = change.m_TargetBinding;
            writeInfo.dstArrayElement = 0;
            writeInfo.descriptorType = change.m_DescriptorType;
            writeInfo.descriptorCount = 1;

            switch (change.m_DescriptorType)
            {
            case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                writeInfo.pBufferInfo = &change.m_BufferUpdate;
                break;
            case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
            case VK_DESCRIPTOR_TYPE_SAMPLER:
                writeInfo.pImageInfo = &change.m_ImageUpdate;
                break;
            default:
                abort();
            }
        }

        vkUpdateDescriptorSets(m_Services.m_LogicalDevice->GetVkDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

        vkCmdBindDescriptorSets(m_VkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CurrentGraphicsPipeline->m_VkPipelineLayout,
            pending.first, 1, &set, 0, nullptr);


        m_IntermediateDescriptorSetAllocations.push_back(std::move(descriptorAlloc));
    }

    m_PendingDescriptorUpdates.clear();
}

void krt::CommandBuffer::TransitionImageLayout(VkImage a_VkImage, VkImageLayout a_OldLayout, VkImageLayout a_NewLayout, VkAccessFlags a_SrcAccessMask, VkAccessFlags
                                               a_DstAccessMask, VkPipelineStageFlags a_SrcStageMask, VkPipelineStageFlags a_DstStageMask)
{
    VkImageMemoryBarrier imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.oldLayout = a_OldLayout;
    imageBarrier.newLayout = a_NewLayout;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = a_VkImage;
    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = 1;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = 1;
    imageBarrier.srcAccessMask = a_SrcAccessMask;
    imageBarrier.dstAccessMask = a_DstAccessMask;

    vkCmdPipelineBarrier(m_VkCommandBuffer, a_SrcStageMask, a_DstStageMask, 0, 0, nullptr,
        0, nullptr, 1, &imageBarrier);
}

std::unique_ptr<krt::VertexBuffer> krt::CommandBuffer::CreateVertexBuffer(void* a_BufferData, uint64_t a_NumElements,
                                                                          uint64_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess)
{
    a_QueuesWithAccess.insert(ETransferQueue);

    uint64_t bufferSize = a_NumElements * a_ElementSize;
    std::unique_ptr<VertexBuffer> buffer = std::make_unique<VertexBuffer>(m_Services);
    buffer->m_NumElements = static_cast<uint32_t>(a_NumElements);

    auto staging = CreateBufferElements(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,{ ETransferQueue });

    auto local = CreateBufferElements(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, a_QueuesWithAccess);

    buffer->m_VkBuffer = local.first;
    buffer->m_VkDeviceMemory = local.second;

    CopyToDeviceMemory(staging.second, a_BufferData, bufferSize);

    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    copyRegion.dstOffset = 0;
    copyRegion.srcOffset = 0;
    vkCmdCopyBuffer(m_VkCommandBuffer, staging.first, buffer->m_VkBuffer, 1, &copyRegion);

    m_IntermediateBuffers.push_back(staging.first);
    m_IntermediateBufferMemoryAllocations.push_back(staging.second);

    return buffer;
}

void krt::CommandBuffer::SetUniformBuffer(const void* a_Data, uint64_t a_DataSize, uint32_t a_Binding, uint32_t a_Set)
{
    auto buffer = CreateBufferElements(a_DataSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, { m_CommandQueue.GetType() });

    CopyToDeviceMemory(buffer.second, a_Data, a_DataSize);

    m_IntermediateBuffers.push_back(buffer.first);
    m_IntermediateBufferMemoryAllocations.push_back(buffer.second);

    auto& update = m_PendingDescriptorUpdates[a_Set].emplace_back();
    update.m_BufferUpdate = {};
    update.m_BufferUpdate.offset = 0;
    update.m_BufferUpdate.buffer = m_IntermediateBuffers.back();
    update.m_BufferUpdate.range = VK_WHOLE_SIZE;
    update.m_TargetBinding = a_Binding;
    update.m_DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

}

std::pair<VkBuffer, VkDeviceMemory> krt::CommandBuffer::CreateBufferElements(uint64_t a_Size, VkBufferUsageFlags a_Usage,
                                                                             VkMemoryPropertyFlags a_MemoryProperties, const std::set<ECommandQueueType>& a_QueuesWithAccess)
{
    VkBufferCreateInfo bufferInfo = {};

    auto queues = GetQueueIndices(a_QueuesWithAccess);

    std::pair<VkBuffer, VkDeviceMemory> output;

    bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = a_Size;
    bufferInfo.usage = a_Usage;
    bufferInfo.pQueueFamilyIndices = queues.data();
    bufferInfo.queueFamilyIndexCount = queues.size() == 1 ? VK_SHARING_MODE_EXCLUSIVE : VK_SHARING_MODE_CONCURRENT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ThrowIfFailed(vkCreateBuffer(m_Services.m_LogicalDevice->GetVkDevice(), &bufferInfo, m_Services.m_AllocationCallbacks, &output.first));

    auto memoryInfo = m_Services.m_PhysicalDevice->GetMemoryInfoForBuffer(output.first, a_MemoryProperties);

    VkMemoryAllocateInfo alloc = {};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.memoryTypeIndex = memoryInfo.m_MemoryType;
    alloc.allocationSize = memoryInfo.m_Size;

    ThrowIfFailed(vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &alloc, m_Services.m_AllocationCallbacks, &output.second));
    ThrowIfFailed(vkBindBufferMemory(m_Services.m_LogicalDevice->GetVkDevice(), output.first, output.second, 0));

    return output;
}

std::vector<uint32_t> krt::CommandBuffer::GetQueueIndices(const std::set<ECommandQueueType>& a_Queues)
{
    std::set<uint32_t> indicesSet;
    for (auto queue : a_Queues)
    {
        indicesSet.insert(m_Services.m_LogicalDevice->GetCommandQueue(queue).GetFamilyIndex());
    }

    return std::vector<uint32_t>(indicesSet.begin(), indicesSet.end());
}

void krt::CommandBuffer::CopyToDeviceMemory(VkDeviceMemory a_DeviceMemory, const void* a_Data, VkDeviceSize a_Size)
{
    void* mem;
    vkMapMemory(m_Services.m_LogicalDevice->GetVkDevice(), a_DeviceMemory, 0, a_Size, 0, &mem);
    memcpy(mem, a_Data, a_Size);
    vkUnmapMemory(m_Services.m_LogicalDevice->GetVkDevice(), a_DeviceMemory);
}
