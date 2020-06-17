#pragma once

#include <memory>
#include <vector>

namespace krt
{
    class VertexBuffer;
    class IndexBuffer;
    class Texture;
    class Sampler;
    class DescriptorSet;
    class GraphicsPipeline;
}

namespace krt
{
    class Material
    {
    public:

        Material();
        ~Material();

        Material(Material&) = delete;
        Material(Material&&) = delete;

        void SetSampler(const Sampler& a_NewSampler);
        void SetDiffuseTexture(const Texture& a_NewDiffuseTexture);

        const DescriptorSet& GetDescriptorSet(GraphicsPipeline& a_TargetPipeline, uint32_t a_SetIndex) const;

    private:

        void UpdateDescriptorSet(GraphicsPipeline& a_TargetPipeline, uint32_t a_SetIndex) const;
        void BuildDescriptorSet() const;

        const Sampler* m_Sampler;
        const Texture* m_DiffuseTexture;

        mutable std::unique_ptr<DescriptorSet> m_DescriptorSet;
        mutable bool m_DescriptorSetDirty;

        mutable GraphicsPipeline* m_GraphicsPipeline;
    };

    struct Mesh
    {
        Mesh();
        ~Mesh();

        struct Primitive
        {
            std::unique_ptr<VertexBuffer> m_Positions;
            std::unique_ptr<VertexBuffer> m_TexCoords;

            std::unique_ptr<IndexBuffer> m_IndexBuffer;

            std::shared_ptr<Material> m_Material;
        };

        std::vector<Primitive> m_Primitives;

    };
}