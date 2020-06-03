#include "CommandQueue.h"

#include "LogicalDevice.h"
#include "CommandBuffer.h"

#include "ServiceLocator.h"

krt::CommandQueue::CommandQueue(ServiceLocator& a_Services, uint32_t a_QueueFamily, ECommandQueueType a_QueueType)
    : m_Services(a_Services)
    , m_QueueFamilyIndex(a_QueueFamily)
    , m_QueueType(a_QueueType)
{
    vkGetDeviceQueue(m_Services.m_LogicalDevice->GetVkDevice(), m_QueueFamilyIndex, 0, &m_VkQueue);

    VkCommandPoolCreateInfo cmdPoolInfo = {};
    cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmdPoolInfo.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmdPoolInfo.queueFamilyIndex = m_QueueFamilyIndex;

    vkCreateCommandPool(m_Services.m_LogicalDevice->GetVkDevice(), &cmdPoolInfo, m_Services.m_AllocationCallbacks, &m_VkCommandPool);
}

krt::CommandQueue::~CommandQueue()
{
    Flush();

    for (auto& buffer : m_SingleUseCommandBuffers)
    {
        buffer.reset();
    }

    vkDestroyCommandPool(m_Services.m_LogicalDevice->GetVkDevice(), m_VkCommandPool, m_Services.m_AllocationCallbacks);
    
}

void krt::CommandQueue::Flush()
{
    vkQueueWaitIdle(m_VkQueue);
}

void krt::CommandQueue::SubmitCommandBuffer(CommandBuffer& a_CommandBuffer)
{
    a_CommandBuffer.End();

    auto buffer = a_CommandBuffer.GetVkCommandBuffer();
    auto signalSemaphores = std::vector<VkSemaphore>(a_CommandBuffer.GetSignalSemaphores().begin(), a_CommandBuffer.GetSignalSemaphores().end());
    auto waitSemaphores = std::vector<VkSemaphore>(a_CommandBuffer.GetWaitSemaphores().begin(), a_CommandBuffer.GetWaitSemaphores().end());
    auto waitStageMask = a_CommandBuffer.GetWaitStagesMask();

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &buffer;
    submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
    submitInfo.pSignalSemaphores = signalSemaphores.data();
    submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
    submitInfo.pWaitSemaphores = waitSemaphores.data();
    submitInfo.pWaitDstStageMask = &waitStageMask;

    auto fence = GetUnusedFence();
    vkQueueSubmit(m_VkQueue, 1, &submitInfo, fence);

    auto& pending = m_PendingCommandBuffers.emplace();
    pending.m_CommandBuffers.push_back(&a_CommandBuffer);
    pending.m_SyncFence = fence;
}


krt::CommandBuffer& krt::CommandQueue::GetSingleUseCommandBuffer()
{
    UpdateCommandBufferQueues();

    if (m_AvailableCommandBuffers.empty())
    {
        auto& buffer = m_SingleUseCommandBuffers.emplace_back(std::make_unique<CommandBuffer>(m_Services, *this));
        return *buffer;
    }

    auto* buffer = m_AvailableCommandBuffers.front();
    m_AvailableCommandBuffers.pop();
    return *buffer;
}

VkCommandPool krt::CommandQueue::GetVkCommandPool()
{
    return m_VkCommandPool;
}

VkQueue krt::CommandQueue::GetVkQueue()
{
    return m_VkQueue;
}

void krt::CommandQueue::UpdateCommandBufferQueues()
{
    while (!m_PendingCommandBuffers.empty())
    {
        auto front = m_PendingCommandBuffers.front();
        auto fenceStatus = vkGetFenceStatus(m_Services.m_LogicalDevice->GetVkDevice(), front.m_SyncFence);
        if (fenceStatus == VK_SUCCESS)
        {
            m_PendingCommandBuffers.pop();
            for (auto& commandBuffer : front.m_CommandBuffers)
            {
                m_AvailableCommandBuffers.push(commandBuffer);
                commandBuffer->Reset();
            }

            vkResetFences(m_Services.m_LogicalDevice->GetVkDevice(), 1, &front.m_SyncFence);
            m_AvailableFences.push(front.m_SyncFence);
        }
        else
        {
            break;
        }
    }
}

VkFence krt::CommandQueue::GetUnusedFence()
{
    if (m_AvailableFences.empty())
    {
        VkFence fence;
        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        vkCreateFence(m_Services.m_LogicalDevice->GetVkDevice(), &fenceInfo, m_Services.m_AllocationCallbacks, &fence);

        return fence;
    }

    VkFence fence = m_AvailableFences.front();
    m_AvailableFences.pop();

    return fence;
}



