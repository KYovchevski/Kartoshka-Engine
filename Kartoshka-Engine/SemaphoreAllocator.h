#pragma once

#include <memory>

#include "vulkan/vulkan.h"

#include <vector>

namespace krt
{
    struct ServiceLocator;
    class SemaphoreAllocator;
    class SemaphoreHandle;
    // Shorthand for shared pointer to a SemaphoreHandle
    using Semaphore = std::shared_ptr<SemaphoreHandle>;
}

namespace krt
{
    // Handle class for the semaphores of the semaphore allocator so that
    // out-of-scope semaphores are automatically returned for reuse
    class SemaphoreHandle
    {
    public:
        SemaphoreHandle(VkSemaphore a_Semaphore, SemaphoreAllocator& a_Allocator);
        ~SemaphoreHandle();

        // The handle is supposed to be unique, so no copy c-tor or copy assignment operator
        SemaphoreHandle(SemaphoreHandle& a_Other) = delete;
        SemaphoreHandle& operator=(SemaphoreHandle& a_Other) = delete;

        // Moving the handle however is allowed
        SemaphoreHandle(SemaphoreHandle&& a_Other);
        SemaphoreHandle& operator=(SemaphoreHandle&& a_Other);

        VkSemaphore operator*() const;

    private:
        VkSemaphore m_Semaphore;
        SemaphoreAllocator* m_SemaphoreAllocator;
    };

    // Class with the purpose of providing unused semaphores on demand for synchronization
    class SemaphoreAllocator
    {
    public:
        explicit SemaphoreAllocator(ServiceLocator& a_Services);
        ~SemaphoreAllocator();

        SemaphoreAllocator(SemaphoreAllocator&) = delete;
        SemaphoreAllocator(SemaphoreAllocator&&) = delete;
        SemaphoreAllocator& operator=(SemaphoreAllocator&) = delete;
        SemaphoreAllocator& operator=(SemaphoreAllocator&&) = delete;

        // Gets a semaphore which is not currently used anywhere
        Semaphore GetSemaphore();

        // Returns the semaphore to the allocator for future reuse. Do not call manually
        void ReturnSemaphore(VkSemaphore a_Semaphore);
    private:
        ServiceLocator& m_Services;

        VkSemaphore MakeSemaphore();

        std::vector<VkSemaphore> m_UnusedSemaphores;    // The semaphores which can be put to use immediately
        std::vector<VkSemaphore> m_InUseSemaphores; // Semaphores which are being used somewhere in the program currently
    };
}
