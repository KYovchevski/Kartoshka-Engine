#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace fx
{
    namespace gltf
    {
        struct Document;
        struct Accessor;
    }
}

namespace krt
{
    struct ServiceLocator;
    struct Mesh;
    class VertexBuffer;
    class IndexBuffer;
    class Material;
    class Texture;
    class Sampler;
}

namespace krt
{
    class ModelManager
    {
    public:
        ModelManager(ServiceLocator& a_Services);
        ~ModelManager();

        ModelManager(ModelManager&) = delete;
        ModelManager(ModelManager&&) = delete;
        ModelManager& operator=(ModelManager&) = delete;
        ModelManager& operator=(ModelManager&&) = delete;

        Mesh* LoadModel(std::string a_Path);

    private:

        static uint32_t GetAttributeSize(fx::gltf::Accessor& a_Accessor);
        static uint32_t GetComponentCount(fx::gltf::Accessor& a_Accessor);
        static uint32_t GetComponentSize(fx::gltf::Accessor& a_Accessor);

        template<typename AttributeType>
        std::unique_ptr<VertexBuffer> LoadVertexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const;
        std::unique_ptr<VertexBuffer> MakeVertexBuffer(std::vector<uint8_t>& a_Data, uint32_t a_AttributeSize) const;

        std::unique_ptr<IndexBuffer> LoadIndexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex);

        std::vector<uint8_t> LoadRawData(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const;

        std::unique_ptr<Material> LoadMaterial(fx::gltf::Document a_Doc, std::string a_Filepath, uint32_t a_MaterialIndex);

        ServiceLocator& m_Services;

        std::map<std::string, std::unique_ptr<Mesh>> m_LoadedModels;

        std::vector<std::unique_ptr<VertexBuffer>>  m_LoadedVertexBuffers;
        std::vector<std::unique_ptr<IndexBuffer>>   m_LoadedIndexBuffers;
        std::vector<std::unique_ptr<Material>>      m_LoadedMaterials;
        std::vector<std::unique_ptr<Texture>>      m_LoadedTextures;

        std::unique_ptr<Sampler> m_DefaultSampler;

    };

    
}

#include "ModelManager.inl"
