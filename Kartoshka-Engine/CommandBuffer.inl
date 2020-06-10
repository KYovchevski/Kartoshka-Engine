

template <typename AttributeType>
std::unique_ptr<VertexBuffer> krt::CommandBuffer::CreateVertexBuffer(std::vector<AttributeType> a_BufferElements, std::set<ECommandQueueType> a_QueuesWithAccess)
{
    return CreateVertexBuffer(a_BufferElements.data(), a_BufferElements.size(), sizeof(AttributeType), a_QueuesWithAccess);
}

template <typename IndexType>
std::unique_ptr<IndexBuffer> CommandBuffer::CreateIndexBuffer(std::vector<IndexType> a_Indices,
    std::set<ECommandQueueType> a_QueuesWithAccess)
{
    static_assert(sizeof(IndexType) <= 4);
    return CreateIndexBuffer(a_Indices.data(), a_Indices.size(), static_cast<uint8_t>(sizeof(IndexType)), a_QueuesWithAccess);
}

template <typename DataType>
void CommandBuffer::SetUniformBuffer(const DataType& a_Data, uint32_t a_Binding, uint32_t a_Set)
{
    SetUniformBuffer(&a_Data, sizeof(DataType), a_Binding, a_Set);
}

template <typename UniformType>
void CommandBuffer::PushConstant(const UniformType& a_Uniform, uint32_t a_Slot)
{
    PushConstant(&a_Uniform, static_cast<uint32_t>(sizeof(UniformType)), a_Slot);
}

