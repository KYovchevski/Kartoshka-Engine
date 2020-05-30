

template <typename AttributeType>
std::unique_ptr<VertexBuffer> krt::CommandBuffer::CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess)
{
    return CreateVertexBuffer(a_BufferElements.data(), a_BufferElements.size(), sizeof(AttributeType), a_QueuesWithAccess);
}