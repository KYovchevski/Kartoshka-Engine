
template<typename BufferType>
std::unique_ptr<BufferType> krt::LogicalDevice::CreateBuffer(uint64_t a_Size, VkBufferUsageFlags a_Usage,
    VkMemoryPropertyFlags a_MemoryProperties, const std::set<ECommandQueueType>& a_QueuesWithAccess)
{
    static_assert(std::is_base_of<Buffer, BufferType>());

    auto elements = CreateBufferElements(a_Size, a_Usage, a_MemoryProperties, a_QueuesWithAccess);

    auto buffer = std::make_unique<BufferType>(m_Services);
    buffer->m_VkBuffer = elements.first;
    buffer->m_VkDeviceMemory = elements.second;

    return std::move(buffer);
}