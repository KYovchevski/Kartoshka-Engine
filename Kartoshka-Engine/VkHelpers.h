#pragma once

#include "vulkan/vulkan.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace krt
{
    // Shorthand to check if a VkResult is VK_SUCCESS and notify the user if it isn't
    void ThrowIfFailed(VkResult a_Result)
    {
        // Don't want these checks running in release, because they shouldn't be needed
#ifdef _DEBUG
        if (a_Result != VK_SUCCESS)
        {
            printf("Vulkan error: %d", a_Result);
            abort();
        }
#endif
    }
}