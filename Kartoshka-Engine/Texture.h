#pragma once

#include "vulkan/vulkan.h"

namespace krt
{
    class CommandBuffer;
    struct ServiceLocator;
}

namespace krt
{
    class Texture
    {
        friend CommandBuffer;
    public:
        Texture(ServiceLocator& a_Services);

        Texture(Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture&) = delete;
        Texture& operator=(Texture&&) = delete;

        ~Texture();

    private:

        ServiceLocator& m_Services;

        VkImage m_VkImage;
        VkDeviceMemory m_VkDeviceMemory;
        VkImageView m_VkImageView;

    };
}
