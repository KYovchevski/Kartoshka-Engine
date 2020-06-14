#include "ModelManager.h"
#include "Mesh.h"

#include "ServiceLocator.h"
#include "LogicalDevice.h"
#include "CommandQueue.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Sampler.h"
#include "Texture.h"

#include "FX-GLTF/gltf.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>


krt::ModelManager::ModelManager(ServiceLocator& a_Services)
    : m_Services(a_Services)
{
    Sampler::CreateInfo info = Sampler::CreateInfo::CreateDefault();
    m_DefaultSampler = std::make_unique<Sampler>(m_Services, info);
}

krt::ModelManager::~ModelManager()
{
}

krt::Mesh* krt::ModelManager::LoadModel(std::string a_Path)
{
    auto doc = fx::gltf::LoadFromText(a_Path);

    auto mesh = doc.meshes[0];

    std::unique_ptr<Mesh> m = std::make_unique<Mesh>();

    for (auto& primitive : mesh.primitives)
    {
        Mesh::Primitive& prim = m->m_Primitives.emplace_back();
        for (auto& attribute : primitive.attributes)
        {
            if (attribute.first == "POSITION")
            {
                m_LoadedVertexBuffers.emplace_back(LoadVertexBuffer<glm::vec3>(doc, attribute.second));
                prim.m_Positions = m_LoadedVertexBuffers.back().get();
            }
            else if (attribute.first == "TEXCOORD_0")
            {
                m_LoadedVertexBuffers.emplace_back(LoadVertexBuffer<glm::vec2>(doc, attribute.second));
                prim.m_TexCoords = m_LoadedVertexBuffers.back().get();
            }
        }

        m_LoadedIndexBuffers.emplace_back(LoadIndexBuffer(doc, primitive.indices));
        prim.m_IndexBuffer = m_LoadedIndexBuffers.back().get();


        auto mat = LoadMaterial(doc, a_Path, primitive.material);
        m_LoadedMaterials.push_back(std::move(mat));
        prim.m_Material = m_LoadedMaterials.back().get();

    }

    m_LoadedModels.emplace(a_Path, std::move(m));

    return m_LoadedModels[a_Path].get();
}

uint32_t krt::ModelManager::GetAttributeSize(fx::gltf::Accessor& a_Accessor)
{
    return GetComponentCount(a_Accessor) * GetComponentSize(a_Accessor);
}

uint32_t krt::ModelManager::GetComponentCount(fx::gltf::Accessor& a_Accessor)
{
    switch (a_Accessor.type)
    {
    case fx::gltf::Accessor::Type::Scalar:
        return 1;
    case fx::gltf::Accessor::Type::Vec2:
        return 2;
    case fx::gltf::Accessor::Type::Vec3:
        return 3;
    case fx::gltf::Accessor::Type::Vec4:
    case fx::gltf::Accessor::Type::Mat2:
        return 4;
    case fx::gltf::Accessor::Type::Mat3:
        return 9;
    case fx::gltf::Accessor::Type::Mat4:
        return 16;
    default:
        assert(0 && "Failed to load GLTF file.");
        return 0;
    }
}

uint32_t krt::ModelManager::GetComponentSize(fx::gltf::Accessor& a_Accessor)
{
    switch (a_Accessor.componentType)
    {
    case fx::gltf::Accessor::ComponentType::Byte:
    case fx::gltf::Accessor::ComponentType::UnsignedByte:
        return 1;
    case fx::gltf::Accessor::ComponentType::Short:
    case fx::gltf::Accessor::ComponentType::UnsignedShort:
        return 2;
    case fx::gltf::Accessor::ComponentType::Float:
    case fx::gltf::Accessor::ComponentType::UnsignedInt:
        return 4;
    default:
        assert(0 && "Failed to load GLTF file.");
        return 0;
    }
}

std::unique_ptr<krt::VertexBuffer> krt::ModelManager::MakeVertexBuffer(std::vector<uint8_t>& a_Data,
                                                                       uint32_t a_AttributeSize) const
{
    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetSingleUseCommandBuffer();

    std::vector<float> fs;

    for (size_t i = 0; i < a_Data.size() / 4; i++)
    {
        union u
        {
            uint8_t b[4];
            float f;
        } u;
        for (size_t j = 0; j < 4; j++)
        {
            u.b[j] = a_Data[i * 4 + j];
        }

        fs.push_back(u.f);
    }

    commandBuffer.Begin();
    auto buffer = commandBuffer.CreateVertexBuffer(a_Data.data(), a_Data.size() / a_AttributeSize,
        a_AttributeSize, { EGraphicsQueue });

    commandBuffer.Submit();

    return buffer;
}

std::unique_ptr<krt::IndexBuffer> krt::ModelManager::LoadIndexBuffer(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex)
{
    if (a_AccessorIndex == -1)
    {
        return nullptr;
    }
    auto accessor = a_Doc.accessors[a_AccessorIndex];

    auto indexSize = GetAttributeSize(accessor);

    auto data = LoadRawData(a_Doc, a_AccessorIndex);

    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetSingleUseCommandBuffer();

    commandBuffer.Begin();
    auto indexBuffer = commandBuffer.CreateIndexBuffer(data.data(), 
        data.size() / indexSize, static_cast<uint8_t>(indexSize), { EGraphicsQueue });

    commandBuffer.Submit();

    return indexBuffer;
}

std::vector<uint8_t> krt::ModelManager::LoadRawData(fx::gltf::Document& a_Doc, int32_t a_AccessorIndex) const
{
    auto accessor = a_Doc.accessors[a_AccessorIndex];
    auto bufferView = a_Doc.bufferViews[accessor.bufferView];

    auto attributeSize = GetAttributeSize(accessor);
    auto componentCount = GetComponentCount(accessor);
    auto componentSize = GetComponentSize(accessor);

    auto buffer = a_Doc.buffers[bufferView.buffer];

    // According to the GLTF spec, BufferView::byteStride can be 0.
    // In those cases the expected stride is the size of the attributes instead.
    auto stride = std::max(attributeSize, bufferView.byteStride);

    std::vector<unsigned char> data;
    // The attribute count is the length of the buffer divided by the stride.
    // Knowing the size of the attributes we can determine how many bytes are needed.
    data.reserve(bufferView.byteLength / stride * attributeSize);

    auto bufferOffset = bufferView.byteOffset + accessor.byteOffset;
    //auto endOffset = bufferOffset + bufferView.byteLength;

    for(uint32_t i = 0; i < accessor.count; i++)
    {
        for (uint32_t component = 0; component < componentCount; component++)
        {
            for (size_t byte = 0; byte < componentSize; byte++)
            {
                data.push_back(buffer.data[bufferOffset + (component * componentSize) + byte]);
            }
        }
        
        bufferOffset += stride;
    }

    //std::reverse(data.begin(), data.end());

    return data;
}

std::unique_ptr<krt::Material> krt::ModelManager::LoadMaterial(fx::gltf::Document a_Doc, std::string a_Filepath, uint32_t a_MaterialIndex)
{
    if (a_MaterialIndex < 0)
        return nullptr;

    std::unique_ptr<Material> m = std::make_unique<Material>();

    m->SetSampler(*m_DefaultSampler);

    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetSingleUseCommandBuffer();
    commandBuffer.Begin();

    auto mat = a_Doc.materials[a_MaterialIndex];
    auto diffuse = mat.pbrMetallicRoughness.baseColorTexture;

    if (!diffuse.empty())
    {
        auto image = a_Doc.images[diffuse.index];

        // Don't want to support embedded textures yet, throw and assert if it's an embedded texture
        assert(image.IsEmbeddedResource() || !image.uri.empty());

        auto texPath = a_Filepath + "/../" + image.uri;

        m_LoadedTextures.emplace_back(commandBuffer.CreateTextureFromFile(texPath, { EGraphicsQueue }));
        auto tex = m_LoadedTextures.back().get();

        m->SetDiffuseTexture(*tex);
    }

    commandBuffer.Submit();

    return m;
}


