#include "CommandBuffer.h"

#include "ServiceLocator.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "CommandQueue.h"
#include "VertexBuffer.h"
#include "RenderPass.h"
#include "GraphicsPipeline.h"

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

    for (auto memory : m_StagingBufferMemoryAllocations)
    {
        vkFreeMemory(m_Services.m_LogicalDevice->GetVkDevice(), memory, m_Services.m_AllocationCallbacks);
    }

    m_StagingBufferMemoryAllocations.clear();

    for (auto stagingBuffer : m_StagingBuffers)
    {
        vkDestroyBuffer(m_Services.m_LogicalDevice->GetVkDevice(), stagingBuffer, m_Services.m_AllocationCallbacks);
    }

    m_StagingBuffers.clear();
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
}

void krt::CommandBuffer::Draw(uint32_t a_NumVertices, uint32_t a_NumInstances, uint32_t a_FirstVertex,
    uint32_t a_FirstInstance)
{
    vkCmdDraw(m_VkCommandBuffer, a_NumVertices, a_NumInstances, a_FirstVertex, a_FirstInstance);
}

void krt::CommandBuffer::DrawIndexed(uint32_t a_NumIndices, uint32_t a_NumInstances, uint32_t a_FirstIndex,
    uint32_t a_FirstInstance, uint32_t a_VertexOffset)
{
    vkCmdDrawIndexed(m_VkCommandBuffer, a_NumIndices, a_NumInstances, a_FirstIndex, a_VertexOffset, a_FirstInstance);
}


VkCommandBuffer krt::CommandBuffer::GetVkCommandBuffer()
{
    return m_VkCommandBuffer;
}

std::unique_ptr<krt::VertexBuffer> krt::CommandBuffer::CreateVertexBuffer(void* a_BufferData, uint64_t a_NumElements,
    uint64_t a_ElementSize, std::set<ECommandQueueType> a_QueuesWithAccess)
{
    std::set<uint32_t> queueIndicesSet;
    // The transfer queue is always necessary
    uint32_t transferQueueIndex = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetFamilyIndex();
    queueIndicesSet.insert(transferQueueIndex); 
    for (auto queue : a_QueuesWithAccess)
    {
        queueIndicesSet.insert(m_Services.m_LogicalDevice->GetCommandQueue(queue).GetFamilyIndex());
    }
    std::vector<uint32_t> queueIndices = std::vector<uint32_t>(queueIndicesSet.begin(), queueIndicesSet.end());

    uint64_t bufferSize = a_NumElements * a_ElementSize;
    std::unique_ptr<VertexBuffer> buffer = std::make_unique<VertexBuffer>(m_Services);
    buffer->m_NumElements = static_cast<uint32_t>(a_NumElements);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.pQueueFamilyIndices = &transferQueueIndex;
    bufferInfo.queueFamilyIndexCount = 1;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = bufferSize;
    ThrowIfFailed(vkCreateBuffer(m_Services.m_LogicalDevice->GetVkDevice(), &bufferInfo, m_Services.m_AllocationCallbacks, &stagingBuffer));

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.pQueueFamilyIndices = queueIndices.data();
    bufferInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueIndices.size());
    bufferInfo.sharingMode = queueIndices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    ThrowIfFailed(vkCreateBuffer(m_Services.m_LogicalDevice->GetVkDevice(), &bufferInfo, m_Services.m_AllocationCallbacks, &buffer->m_VkBuffer));

    auto stagingBufferMemoryType = m_Services.m_PhysicalDevice->FindMemoryType(stagingBuffer, 
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    auto localBufferMemoryType = m_Services.m_PhysicalDevice->FindMemoryType(stagingBuffer, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(m_Services.m_LogicalDevice->GetVkDevice(), stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = stagingBufferMemoryType;
    ThrowIfFailed(vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, m_Services.m_AllocationCallbacks, &stagingBufferMemory));
    ThrowIfFailed(vkBindBufferMemory(m_Services.m_LogicalDevice->GetVkDevice(), stagingBuffer, stagingBufferMemory, 0));

    vkGetBufferMemoryRequirements(m_Services.m_LogicalDevice->GetVkDevice(), buffer->m_VkBuffer, &memReq);

    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = localBufferMemoryType;
    ThrowIfFailed(vkAllocateMemory(m_Services.m_LogicalDevice->GetVkDevice(), &allocInfo, m_Services.m_AllocationCallbacks, &buffer->m_VkDeviceMemory));
    ThrowIfFailed(vkBindBufferMemory(m_Services.m_LogicalDevice->GetVkDevice(), buffer->m_VkBuffer, buffer->m_VkDeviceMemory, 0));

    void* mappedMemory;
    vkMapMemory(m_Services.m_LogicalDevice->GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedMemory);
    memcpy(mappedMemory, a_BufferData, bufferSize);
    vkUnmapMemory(m_Services.m_LogicalDevice->GetVkDevice(), stagingBufferMemory);

    VkBufferCopy copyRegion = {};
    copyRegion.size = bufferSize;
    copyRegion.dstOffset = 0;
    copyRegion.srcOffset = 0;
    vkCmdCopyBuffer(m_VkCommandBuffer, stagingBuffer, buffer->m_VkBuffer, 1, &copyRegion);

    //m_StagingBuffers.push_back(stagingBuffer);
    //m_StagingBufferMemoryAllocations.push_back(stagingBufferMemory);

    return buffer;
}

