#include "Mesh.h"
#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "DescriptorSet.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

krt::Material::Material()
    : m_Sampler(nullptr)
    , m_DiffuseTexture(nullptr)
    , m_DescriptorSet(nullptr)
    , m_DescriptorSetDirty(true)
    , m_GraphicsPipeline(nullptr)
{
    m_DescriptorSet = nullptr;
}

krt::Material::~Material()
{
    printf("dest");
}

void krt::Material::SetSampler(const Sampler& a_NewSampler)
{
    m_Sampler = &a_NewSampler;
    m_DescriptorSetDirty = true;
}

void krt::Material::SetDiffuseTexture(const std::shared_ptr<Texture> a_NewDiffuseTexture)
{
    m_DiffuseTexture = a_NewDiffuseTexture;
    m_DescriptorSetDirty = true;
}

void krt::Material::SetNormalMap(const std::shared_ptr<Texture> a_NewNormalMap)
{
    m_NormalMap = a_NewNormalMap;
    m_DescriptorSetDirty = true;
}

void krt::Material::SetDiffuseColor(const glm::vec4& a_NewDiffuseColor)
{
    m_DiffuseColor = a_NewDiffuseColor;
    m_DescriptorSetDirty = true;
}

krt::DescriptorSet& krt::Material::GetDescriptorSet(GraphicsPipeline& a_TargetPipeline, uint32_t a_SetIndex)
{
    UpdateDescriptorSet(a_TargetPipeline, a_SetIndex);
    return *m_DescriptorSet;
}

void krt::Material::UpdateDescriptorSet(GraphicsPipeline& a_TargetPipeline, uint32_t a_SetIndex) const
{
    if (!m_DescriptorSetDirty && m_GraphicsPipeline == &a_TargetPipeline)
        return;

    if (m_DescriptorSet == nullptr || m_GraphicsPipeline != &a_TargetPipeline)
    {
        m_DescriptorSet = a_TargetPipeline.CreateDescriptorSet(a_SetIndex, { EGraphicsQueue });
        m_GraphicsPipeline = &a_TargetPipeline;
    }
    BuildDescriptorSet();
}

void krt::Material::BuildDescriptorSet() const
{
    m_DescriptorSet->SetSampler(*m_Sampler, 0);
    m_DescriptorSet->SetTexture(*m_DiffuseTexture, 1);
    m_DescriptorSet->SetUniformBuffer(m_DiffuseColor, 2, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    m_DescriptorSet->SetTexture(*m_NormalMap, 3);
    m_DescriptorSetDirty = false;
}

krt::Mesh::Mesh()
{
}

krt::Mesh::~Mesh()
{
}
