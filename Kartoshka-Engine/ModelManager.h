#pragma once

#include "FX-GLTF/gltf.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace krt
{
    class Transform;
    struct ServiceLocator;
    struct Mesh;
    class VertexBuffer;
    class IndexBuffer;
    class Material;
    class Texture;
    class Sampler;
    class Scene;
    class StaticMesh;
}

namespace krt
{
    class ModelManager
    {
    public:

        class GLTFResource
        {
            friend ModelManager;
        public:

            std::shared_ptr<Mesh> GetMesh(uint32_t a_Index = 0) { return m_Meshes[a_Index]; }

            std::shared_ptr<Scene> GetScene(uint32_t a_Index = 0) { return m_Scenes[a_Index]; }

        private:
            std::vector<std::shared_ptr<Mesh>>      m_Meshes;
            std::vector<std::shared_ptr<Material>>  m_Materials;
            std::vector<std::shared_ptr<Texture>>   m_LoadedTextures;
            std::vector<std::shared_ptr<Scene>>     m_Scenes;

        };

        ModelManager(ServiceLocator& a_Services);
        ~ModelManager();

        ModelManager(ModelManager&) = delete;
        ModelManager(ModelManager&&) = delete;
        ModelManager& operator=(ModelManager&) = delete;
        ModelManager& operator=(ModelManager&&) = delete;

        GLTFResource* LoadGltf(std::string a_Path);

    private:

        static uint32_t GetAttributeSize(fx::gltf::Accessor& a_Accessor);
        static uint32_t GetComponentCount(fx::gltf::Accessor& a_Accessor);
        static uint32_t GetComponentSize(fx::gltf::Accessor& a_Accessor);

        std::vector<std::shared_ptr<Texture>> LoadTextures(fx::gltf::Document& a_Doc, std::string a_Filepath);
        std::vector<std::shared_ptr<Material>> LoadMaterials(fx::gltf::Document& a_Doc, GLTFResource& a_Res);
        std::vector<std::shared_ptr<Mesh>> LoadMeshes(fx::gltf::Document& a_Doc, GLTFResource& a_Res);
        std::vector<std::shared_ptr<Scene>> LoadScenes(fx::gltf::Document& a_Doc, GLTFResource& a_Res);

        void LoadNode(fx::gltf::Document& a_Doc, GLTFResource& a_Res, int32_t a_NodeIndex, const Transform& a_NodeParent,
                      Scene& a_Scene);

        void GetNodeTransform(fx::gltf::Node& a_Node, Transform& a_Transform);

        template<typename AttributeType>
        std::unique_ptr<VertexBuffer> LoadVertexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const;
        std::unique_ptr<VertexBuffer> MakeVertexBuffer(std::vector<uint8_t>& a_Data, uint32_t a_AttributeSize) const;

        std::unique_ptr<IndexBuffer> LoadIndexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex);

        std::vector<uint8_t> LoadRawData(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const;

        ServiceLocator& m_Services;

        std::map<std::string, GLTFResource> m_LoadedGLTFs;

        std::unique_ptr<Sampler> m_DefaultSampler;

    };

    
}

#include "ModelManager.inl"
