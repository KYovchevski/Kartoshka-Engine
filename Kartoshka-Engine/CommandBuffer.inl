

template <typename AttributeType>
std::unique_ptr<VertexBuffer> krt::CommandBuffer::CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess)
{
    return CreateVertexBuffer(a_BufferElements.data(), a_BufferElements.size(), sizeof(AttributeType), a_QueuesWithAccess);
}

template <typename DataType>
void CommandBuffer::SetUniformBuffer(const DataType& a_Data, uint32_t a_Binding, uint32_t a_Set)
{
    SetUniformBuffer(&a_Data, sizeof(DataType), a_Binding, a_Set);
}
