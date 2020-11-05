#include "ResourceStateTracker.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"

krt::ResourceStateTracker::ResourceStateTracker(ServiceLocator& a_ServiceLocator)
    : m_Services(a_ServiceLocator)
{
}

void krt::ResourceStateTracker::SynchronizeImageStates(const std::unordered_map<VkImage, ImageState>& a_ChangedStates)
{
    auto& cmdBuffer = m_Services.m_LogicalDevice->GetCommandQueue(EGraphicsQueue).GetSingleUseCommandBuffer();

    cmdBuffer.Begin();
    for (auto& changedState : a_ChangedStates)
    {
        auto& image = changedState.first;
        auto& newState = changedState.second;

        if (m_Images.count(image) != 0)
        {
            auto& oldState = m_Images[image];
            VkImageMemoryBarrier barr = {};
            barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barr.image = image;
            barr.srcAccessMask = oldState.m_CurrentAccessMask;
            barr.dstAccessMask = newState.m_FirstAccessMask;
            barr.oldLayout = oldState.m_CurrentLayout;
            barr.newLayout = newState.m_FirstLayout;
            barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barr.subresourceRange = newState.m_SubresourceRange;

            vkCmdPipelineBarrier()
        }
    }
}
