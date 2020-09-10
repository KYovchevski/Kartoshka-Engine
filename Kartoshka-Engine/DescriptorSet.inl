template <typename BufferStruct>
void krt::DescriptorSet::SetUniformBuffer(const BufferStruct& a_BufferStruct, uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    return SetUniformBuffer(&a_BufferStruct, sizeof(BufferStruct), a_Binding, a_UsingStage);
}

template <typename BufferStruct>
void krt::DescriptorSet::SetStorageBuffer(const std::vector<BufferStruct>& a_StructList, uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    return SetStorageBuffer(a_StructList.data(), a_StructList.size() * sizeof(BufferStruct), a_Binding);
}
