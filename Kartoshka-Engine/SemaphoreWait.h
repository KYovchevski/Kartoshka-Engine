#pragma once
#include "SemaphoreAllocator.h"

namespace krt
{
    struct SemaphoreWait
    {
        Semaphore m_Semaphore;
        VkPipelineStageFlags m_StageFlags;
    };
}