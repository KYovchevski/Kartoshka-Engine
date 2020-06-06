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
        Texture(ServiceLocator& a_Services, VkFormat a_Format);

        Texture(Texture&) = delete;
        Texture(Texture&&) = delete;
        Texture& operator=(Texture&) = delete;
        Texture& operator=(Texture&&) = delete;

        ~Texture();

        VkFormat GetVkFormat() const { return m_Format; }
        VkImageView GetVkImageView() const { return m_VkImageView; }

    protected:

        ServiceLocator& m_Services;

        VkImage m_VkImage;
        VkDeviceMemory m_VkDeviceMemory;
        VkImageView m_VkImageView;

        VkFormat m_Format;

    };
}
