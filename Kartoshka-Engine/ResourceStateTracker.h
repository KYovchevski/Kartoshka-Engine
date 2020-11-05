#pragma once

#include "ResourceStateStructs.h"

#include <vulkan/vulkan.h>
#include <unordered_map>

namespace krt
{
    struct ServiceLocator;
}

namespace krt
{
    class ResourceStateTracker
    {
    public:
        ResourceStateTracker(ServiceLocator& a_ServiceLocator);

        void SynchronizeImageStates(const std::unordered_map<VkImage, ImageState>& a_ChangedStates);
        void RegisterImage(VkImage a_VkImage, ImageState& a_ImageState);

    private:

        ServiceLocator& m_Services;

        std::unordered_map<VkImage, ImageState> m_Images;
    };    
}

