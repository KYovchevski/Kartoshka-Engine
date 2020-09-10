#include "SemaphoreAllocator.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::SemaphoreHandle::SemaphoreHandle(VkSemaphore a_Semaphore, SemaphoreAllocator& a_Allocator)
    : m_Semaphore(a_Semaphore)
    , m_SemaphoreAllocator(&a_Allocator)
{
}

krt::SemaphoreHandle::~SemaphoreHandle()
{
    if (m_Semaphore != VK_NULL_HANDLE)
    {
        m_SemaphoreAllocator->ReturnSemaphore(m_Semaphore);
    }
}

krt::SemaphoreHandle::SemaphoreHandle(SemaphoreHandle&& a_Other)
{
    m_Semaphore = a_Other.m_Semaphore;
    m_SemaphoreAllocator = a_Other.m_SemaphoreAllocator;
    a_Other.m_Semaphore = VK_NULL_HANDLE;
}

krt::SemaphoreHandle& krt::SemaphoreHandle::operator=(SemaphoreHandle&& a_Other)
{
    m_Semaphore = a_Other.m_Semaphore;
    m_SemaphoreAllocator = a_Other.m_SemaphoreAllocator;
    a_Other.m_Semaphore = VK_NULL_HANDLE;

    return *this;
}

VkSemaphore krt::SemaphoreHandle::operator*() const
{
    return m_Semaphore;
}


krt::SemaphoreAllocator::SemaphoreAllocator(ServiceLocator& a_Services)
    : m_Services(a_Services)
{

}

krt::SemaphoreAllocator::~SemaphoreAllocator()
{
    // Flush the device before continuing
    m_Services.m_LogicalDevice->Flush();

    // Because the device was flushed, the InUse semaphores aren't actually in use, so they can be destroyed without causing errors
    for (auto semaphore : m_InUseSemaphores)
        vkDestroySemaphore(m_Services.m_LogicalDevice->GetVkDevice(), semaphore, m_Services.m_AllocationCallbacks);

    for (auto unusedSemaphore : m_UnusedSemaphores)
    {
        vkDestroySemaphore(m_Services.m_LogicalDevice->GetVkDevice(), unusedSemaphore, m_Services.m_AllocationCallbacks);
    }
}

krt::Semaphore krt::SemaphoreAllocator::GetSemaphore()
{
    VkSemaphore semaphore;
    if (m_UnusedSemaphores.empty())
    {
        semaphore = MakeSemaphore();
    }
    else
    {
        std::swap(m_UnusedSemaphores.front(), m_UnusedSemaphores.back());
        semaphore = m_UnusedSemaphores.back();
        m_UnusedSemaphores.pop_back();
    }
    m_InUseSemaphores.push_back(semaphore);

    auto sem = std::make_shared<SemaphoreHandle>(semaphore, *this);
    return sem;
}

void krt::SemaphoreAllocator::ReturnSemaphore(VkSemaphore a_Semaphore)
{
    m_UnusedSemaphores.push_back(a_Semaphore);

    auto f = std::find(m_InUseSemaphores.begin(), m_InUseSemaphores.end(), a_Semaphore);
    std::iter_swap(f, m_InUseSemaphores.end() - 1);
    m_InUseSemaphores.pop_back();
}

VkSemaphore krt::SemaphoreAllocator::MakeSemaphore()
{
    VkSemaphore sem;
    VkSemaphoreCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(m_Services.m_LogicalDevice->GetVkDevice(), &info, m_Services.m_AllocationCallbacks, &sem);

    return sem;
}
