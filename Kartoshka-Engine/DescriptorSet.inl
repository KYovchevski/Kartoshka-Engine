template <typename BufferStruct>
void krt::DescriptorSet::SetUniformBuffer(const BufferStruct& a_BufferStruct, uint32_t a_Binding)
{
    SetUniformBuffer(&a_BufferStruct, sizeof(BufferStruct), a_Binding);
}

