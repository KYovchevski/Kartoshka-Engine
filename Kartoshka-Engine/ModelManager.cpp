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
#include "Scene.h"
#include "StaticMesh.h"
#include "Transform.h"

#include "FX-GLTF/gltf.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/matrix_decompose.hpp>


krt::ModelManager::ModelManager(ServiceLocator& a_Services)
    : m_Services(a_Services)
{
    Sampler::CreateInfo info = Sampler::CreateInfo::CreateDefault();
    m_DefaultSampler = std::make_unique<Sampler>(m_Services, info);

    auto& commandBuffer = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue).GetSingleUseCommandBuffer();
    commandBuffer.Begin();

    m_DefaultDiffuse = commandBuffer.CreateTextureFromFile("../../../../Assets/Textures/DiffuseDefault.png", { EGraphicsQueue });

    glm::vec4 defNormal(0.5f, 0.5f, 1.0f, 1.0f);

    m_DefaultNormalMap = commandBuffer.CreateTexture(&defNormal[0], glm::uvec2(1, 1), 4, 1, { EGraphicsQueue });

    commandBuffer.Submit();

}

krt::ModelManager::~ModelManager()
{
}

krt::ModelManager::GLTFResource* krt::ModelManager::LoadGltf(std::string a_Path)
{
    auto findIter = m_LoadedGLTFs.find(a_Path);

    if (findIter != m_LoadedGLTFs.end())
        return &(*findIter).second;

    auto& res = m_LoadedGLTFs[a_Path];

    auto doc = fx::gltf::LoadFromText(a_Path);

    res.m_LoadedTextures = LoadTextures(doc, a_Path);
    res.m_Materials = LoadMaterials(doc, res);
    res.m_Meshes = LoadMeshes(doc, res);
    res.m_Scenes = LoadScenes(doc, res);

    return& res;
}

std::vector<std::shared_ptr<krt::Mesh>> krt::ModelManager::LoadMeshes(fx::gltf::Document& a_Doc, krt::ModelManager::GLTFResource& a_Res)
{
    std::vector<std::shared_ptr<krt::Mesh>> meshes;

    for (auto& fxMesh : a_Doc.meshes)
    {
        auto& mesh = meshes.emplace_back(std::make_shared<Mesh>());
        for (auto& fxPrimitive : fxMesh.primitives)
        {
            auto& prim = mesh->m_Primitives.emplace_back();
            for (auto& attribute : fxPrimitive.attributes)
            {
                if (attribute.first == "POSITION")
                {
                    prim.m_Positions = LoadVertexBuffer<glm::vec3>(a_Doc, attribute.second);
                }
                else if (attribute.first == "TEXCOORD_0")
                {
                    prim.m_TexCoords = LoadVertexBuffer<glm::vec2>(a_Doc, attribute.second);
                }
                else if (attribute.first == "COLOR_0")
                {
                    prim.m_VertexColors = LoadVertexBuffer<glm::vec4>(a_Doc, attribute.second);
                }
                else if (attribute.first == "NORMAL")
                {
                    prim.m_Normals = LoadVertexBuffer<glm::vec3>(a_Doc, attribute.second);
                }
                else if (attribute.first == "TANGENT")
                {
                    prim.m_Tangents = LoadVertexBuffer<glm::vec4>(a_Doc, attribute.second);
                }
            }
            prim.m_IndexBuffer = LoadIndexBuffer(a_Doc, fxPrimitive.indices);

            prim.m_Material = a_Res.m_Materials[fxPrimitive.material];

            if (!prim.m_VertexColors)
            {
                std::vector<float> colors = std::vector<float>(prim.m_Positions->GetElementCount() * 4, 1.0f);
                std::vector<uint8_t> bytes = std::vector<uint8_t>(colors.size() * sizeof(float));

                memcpy(bytes.data(), colors.data(), bytes.size());

                prim.m_VertexColors = MakeVertexBuffer(bytes, static_cast<uint32_t>(sizeof(glm::vec4)));
            }
        }
    }

    return meshes;
}

std::vector<std::shared_ptr<krt::Scene>> krt::ModelManager::LoadScenes(fx::gltf::Document& a_Doc, GLTFResource& a_Res)
{
    std::vector<std::shared_ptr<krt::Scene>> scenes;
    for (auto& fxScene : a_Doc.scenes)
    {
        auto& scene = scenes.emplace_back(std::make_shared<Scene>(m_Services));

        Transform identity;

        for (auto& node : fxScene.nodes)
        {
            LoadNode(a_Doc, a_Res, node, identity, *scene);
        }
    }

    return scenes;
}

void krt::ModelManager::LoadNode(fx::gltf::Document& a_Doc, GLTFResource& a_Res, int32_t a_NodeIndex,
                                 const Transform& a_NodeParent, krt::Scene& a_Scene)
{
    auto& node = a_Doc.nodes[a_NodeIndex];

    glm::vec3 pos = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);

    Transform local;
    GetNodeTransform(node, local);
    local.SetPosition(pos);

    auto worldTransform = a_NodeParent * local;

    if (node.mesh != -1)
    {
        auto& sMesh = a_Scene.m_StaticMeshes.emplace_back(std::make_unique<StaticMesh>());
        *sMesh->m_Transform = worldTransform;
        sMesh->SetMesh(a_Res.m_Meshes[node.mesh]);
    }

    for (auto& child : node.children)
    {
        LoadNode(a_Doc, a_Res, child, worldTransform, a_Scene);
    }
}

void krt::ModelManager::GetNodeTransform(fx::gltf::Node& a_Node, Transform& a_Transform)
{
    auto& mat = a_Node.matrix;

    if (a_Node.matrix != fx::gltf::defaults::IdentityMatrix)
    {
        // Get the transformation from the matrix provided in the file
        glm::mat4 glmMat = glm::mat4(0);
        memcpy(&glmMat[0], &mat[0], mat.size() * sizeof(float));

        glmMat[3][0] *= -1.0f;
        //glmMat[3][3] *= -1.0f;
        a_Transform = glmMat;
       
    }
    else
    {
        // Get the transformation based on the TRS provided in the file
        glm::vec3 pos(-a_Node.translation[0], a_Node.translation[1], a_Node.translation[2]);
        glm::quat rot(a_Node.rotation[3], a_Node.rotation[0], a_Node.rotation[1], a_Node.rotation[2]);
        glm::vec3 scale(a_Node.scale[0], a_Node.scale[1], a_Node.scale[2]);

        a_Transform.SetPosition(pos);
        a_Transform.SetRotation(rot);
        a_Transform.SetScale(scale);        
    }
}

std::vector<std::shared_ptr<krt::Material>> krt::ModelManager::LoadMaterials(fx::gltf::Document& a_Doc, krt::ModelManager::GLTFResource& a_Res)
{
    std::vector<std::shared_ptr<krt::Material>> materials;
    for (auto& material : a_Doc.materials)
    {
        auto& mat = materials.emplace_back(std::make_shared<Material>());
        mat->SetSampler(*m_DefaultSampler);
        if (!material.pbrMetallicRoughness.baseColorTexture.empty())
        {
            mat->SetDiffuseTexture(a_Res.m_LoadedTextures[material.pbrMetallicRoughness.baseColorTexture.index]);
        }
        else
        {
            mat->SetDiffuseTexture(m_DefaultDiffuse);
        }

        glm::vec4 diffuse = glm::vec4(material.pbrMetallicRoughness.baseColorFactor[0], material.pbrMetallicRoughness.baseColorFactor[1],
            material.pbrMetallicRoughness.baseColorFactor[2], material.pbrMetallicRoughness.baseColorFactor[3]);

        mat->SetDiffuseColor(diffuse);

        if (!material.normalTexture.empty())
            mat->SetNormalMap(a_Res.m_LoadedTextures[material.normalTexture.index]);
        else
            mat->SetNormalMap(m_DefaultNormalMap);


    }
    return materials;
}

std::vector<std::shared_ptr<krt::Texture>> krt::ModelManager::LoadTextures(fx::gltf::Document& a_Doc, std::string a_Filepath)
{
    std::vector<std::shared_ptr<Texture>> textures;

    auto& commandQueue = m_Services.m_LogicalDevice->GetCommandQueue(ETransferQueue);
    auto& commandBuffer = commandQueue.GetSingleUseCommandBuffer();
    commandBuffer.Begin();

    for (auto& image : a_Doc.images)
    {
        // Don't want to support embedded textures yet, assert if it's an embedded texture
        assert(image.IsEmbeddedResource() || !image.uri.empty());

        auto texPath = a_Filepath + "/../" + image.uri;

        auto tex = commandBuffer.CreateTextureFromFile(texPath, { EGraphicsQueue });

        textures.emplace_back(tex.release());
    }

    commandBuffer.Submit();
    commandQueue.Flush();

    return textures;
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
