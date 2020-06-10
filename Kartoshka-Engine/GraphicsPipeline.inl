

template <typename AttributeType>
void krt::VertexInputInfo::AddPerVertexAttribute(uint32_t a_Binding, uint32_t a_Location, VkFormat a_Format)
{
    static_assert(sizeof(AttributeType) < 4 * 64);
    auto& attribute = m_Attributes.emplace_back();

    if (m_Bindings.find(a_Binding) == m_Bindings.end())
    {
        m_Bindings[a_Binding] = {};
    }

    auto& binding = m_Bindings[a_Binding];

    attribute.binding = a_Binding;
    attribute.format = a_Format;
    attribute.location = a_Location;
    attribute.offset = binding.stride;

    binding.binding = a_Binding;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    binding.stride += sizeof(AttributeType);
}


template <typename AttributeType>
void krt::VertexInputInfo::AddPerInstanceAttribute(uint32_t a_Binding, uint32_t a_Location, VkFormat a_Format)
{
    static_assert(sizeof(AttributeType) < 4 * 64);
    auto& attribute = m_Attributes.emplace_back();

    if (m_Bindings.find(a_Binding) == m_Bindings.end())
    {
        m_Bindings[a_Binding] = {};
    }

    auto& binding = m_Bindings[a_Binding];

    attribute.binding = a_Binding;
    attribute.format = a_Format;
    attribute.location = a_Location;
    attribute.offset = binding.stride;

    binding.binding = a_Binding;
    binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
    binding.stride += sizeof(AttributeType);
}

template <typename Type>
void PipelineLayoutInfo::AddPushConstantRange(VkShaderStageFlags a_ShaderStage)
{
    VkPushConstantRange range;

    if (m_PushConstantRanges.empty())
    {
        range.offset = 0;
    }
    else
    {
        auto& previous = m_PushConstantRanges[static_cast<uint32_t>(m_PushConstantRanges.size()) - 1];
        range.offset = previous.size + previous.offset;
    }

    auto size = static_cast<uint32_t>(sizeof(Type));
    range.size = static_cast<uint32_t>(std::ceil(static_cast<float>(size) / 4.0f) * 4.0f);
    range.stageFlags = a_ShaderStage;

    m_PushConstantRanges.emplace(static_cast<uint32_t>(m_PushConstantRanges.size()), range);
}


