#pragma once

#include "Texture.h"

#include "vulkan/vulkan.h"

#include <cstdint>
#include <array>

namespace krt
{
    struct ServiceLocator;
}

namespace krt
{
    class CubeShadowMap
        : public Texture
    {
    public:
        CubeShadowMap(ServiceLocator& a_ServiceLocator, uint32_t a_Dimensions = 512, VkFormat a_ImageFormat = VK_FORMAT_D32_SFLOAT);

        std::array<VkImageView, 6> GetDepthViews();

        void TransitionLayoutToShaderRead(VkImageLayout a_OldLayout, VkAccessFlags a_AccessMask);

        const uint32_t  m_Dimensions;
        //const VkFormat  m_VkFormat;
        //const VkFormat  m_ShaderViewFormat;
    private:

        void CreateImage();
        void CreateShaderImageView();
        void CreateDepthImageViews();

        //VkImage                     m_VkImage;
        //VkDeviceMemory              m_VkImageMemory;
        //VkImageView                 m_ShaderImageView;
        std::array<VkImageView, 6>  m_DepthImageViews;
    };    
}

