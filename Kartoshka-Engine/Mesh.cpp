#include "Mesh.h"
#include "GraphicsPipeline.h"
#include "LogicalDevice.h"
#include "DescriptorSet.h"

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

void krt::Material::SetDiffuseTexture(const Texture& a_NewDiffuseTexture)
{
    m_DiffuseTexture = &a_NewDiffuseTexture;
    m_DescriptorSetDirty = true;
}

const krt::DescriptorSet& krt::Material::GetDescriptorSet(GraphicsPipeline& a_TargetPipeline, uint32_t a_SetIndex) const
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
}

krt::Mesh::Mesh()
{
}

krt::Mesh::~Mesh()
{
}
