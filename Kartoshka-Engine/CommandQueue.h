#pragma once


#include "vulkan/vulkan.h"
#include <vector>
#include <queue>
#include <memory>

namespace krt
{
    struct ServiceLocator;
    class CommandBuffer;
}


namespace krt
{

    class CommandQueue
    {
    public:
        CommandQueue(ServiceLocator& a_Services, uint32_t a_QueueFamily);
        ~CommandQueue();

        CommandQueue(CommandQueue&) = delete; // No copy c-tor
        CommandQueue(CommandQueue&&) = delete; // No move c-tor
        CommandQueue& operator=(CommandQueue&) = delete; // No copy assignment
        CommandQueue& operator=(CommandQueue&&) = delete; // No move assignment

        // Puts the thread to sleep until the queue is finished with its operations
        void Flush();

        // Submit a single command buffer
        void SubmitCommandBuffer(CommandBuffer& a_CommandBuffer);

        // Returns a command buffer which is not being used elsewhere in the application or pending execution
        CommandBuffer& GetSingleUseCommandBuffer();

        VkCommandPool GetVkCommandPool();
        VkQueue GetVkQueue();
        uint32_t GetFamilyIndex() const { return m_QueueFamilyIndex; }

    private:

        VkFence GetUnusedFence();
        void UpdateCommandBufferQueues();

        ServiceLocator&     m_Services;

        VkQueue             m_VkQueue;
        VkCommandPool       m_VkCommandPool;
        uint32_t            m_QueueFamilyIndex;

        // List of all command buffers created by the queue
        std::vector<std::unique_ptr<CommandBuffer>> m_SingleUseCommandBuffers;
        // The queue owns multiple fences which are used for the syncronization of the pending command buffers
        std::queue<VkFence> m_AvailableFences;
        // Each time command buffers are submitted, a fence is associated with them
        struct PendingCommandBuffersEntry
        {
            std::vector<CommandBuffer*> m_CommandBuffers;
            VkFence m_SyncFence;
        };

        // Queue of command buffers that are currently being executed on the queue
        std::queue<PendingCommandBuffersEntry>                  m_PendingCommandBuffers;
        // Queue of command buffers that can be used to record new commands to
        std::queue<CommandBuffer*>                  m_AvailableCommandBuffers;
    };

}

