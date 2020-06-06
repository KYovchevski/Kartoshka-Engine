#pragma once

#include "Texture.h"

#include <set>

namespace krt
{
    enum ECommandQueueType : uint8_t;
}

namespace krt
{

    class DepthBuffer : public krt::Texture
    {
    public:
        DepthBuffer(ServiceLocator& a_Services, uint32_t a_Width, uint32_t a_Height, VkFormat a_Format, std::set<ECommandQueueType> a_UsingQueues);
        ~DepthBuffer();


    private:
    };
}

