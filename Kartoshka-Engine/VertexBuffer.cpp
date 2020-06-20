#include "VertexBuffer.h"
#include "ServiceLocator.h"
#include "LogicalDevice.h"

krt::VertexBuffer::VertexBuffer(ServiceLocator& a_Services)
    : Buffer(a_Services)
{
    
}


krt::VertexBuffer::~VertexBuffer()
{
    
}

uint32_t krt::VertexBuffer::GetElementCount()
{
    return m_NumElements;
}
