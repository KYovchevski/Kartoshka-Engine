template <typename BufferStruct>
krt::SemaphoreWait krt::DescriptorSet::SetUniformBuffer(const BufferStruct& a_BufferStruct, uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    return SetUniformBuffer(&a_BufferStruct, sizeof(BufferStruct), a_Binding, a_UsingStage);
}

template <typename StructType>
krt::SemaphoreWait krt::DescriptorSet::SetUniformBuffer(const std::vector<StructType> a_Data, uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    return SetUniformBuffer(a_Data.data(), a_Data.size() * sizeof(StructType), a_Binding, a_UsingStage);
}


template <typename BufferStruct>
krt::SemaphoreWait krt::DescriptorSet::SetStorageBuffer(const std::vector<BufferStruct>& a_StructList,
                                                        uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    return SetStorageBuffer(a_StructList.data(), a_StructList.size() * sizeof(BufferStruct), a_Binding, a_UsingStage);
}

template <typename HeaderStruct, typename BufferStruct>
krt::SemaphoreWait krt::DescriptorSet::SetStorageBuffer(const HeaderStruct& a_Header, const std::vector<BufferStruct>& a_StructList,
                                                        uint32_t a_Binding, VkPipelineStageFlags a_UsingStage)
{
    std::vector<uint8_t> rawData(sizeof(HeaderStruct) + a_StructList.size() * sizeof(BufferStruct));

    memcpy(rawData.data(), &a_Header, sizeof(HeaderStruct));
    memcpy(rawData.data() + sizeof(HeaderStruct), a_StructList.data(), a_StructList.size() * sizeof(BufferStruct));

    return SetStorageBuffer(rawData, a_Binding, a_UsingStage);
}


