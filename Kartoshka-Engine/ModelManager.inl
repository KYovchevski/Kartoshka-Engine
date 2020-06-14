
template <typename AttributeType>
std::unique_ptr<krt::VertexBuffer> krt::ModelManager::LoadVertexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const
{
    if (a_AccessorIndex >= 0)
    {
        auto data = LoadRawData(a_Doc, a_AccessorIndex);

        return MakeVertexBuffer(data, static_cast<uint32_t>(sizeof(AttributeType)));
    }
    return nullptr;
}