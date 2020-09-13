#include <ivulk/render/model/static_model.hpp>

#include <ivulk/core/app.hpp>
#include <ivulk/utils/messages.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

namespace ivulk {
    namespace fs = utils::fs;

    ///////////////////////////////////////////////////////////////////////
    //                               Mesh                                //
    ///////////////////////////////////////////////////////////////////////

    StaticMesh::StaticMesh(Buffer::Ptr vBuf, Buffer::Ptr iBuf, uint32_t pipelineIndex)
        : m_vertexBuffer(vBuf)
        , m_indexBuffer(iBuf)
        , m_pipelineIndex(pipelineIndex)
    { }

    StaticMesh::Ptr StaticMesh::create(const std::vector<vertex_t>& vertices, const std::vector<uint32_t>& indices, uint32_t pipelineIndex)
    {
        auto state = App::current()->getState().vk;
        Buffer::Ptr vBuf, iBuf;

        // ################# Vertex Buffer ################## //

        {
            VkDeviceSize sz = sizeof(vertices[0]) * vertices.size();
            auto stageBuf   = Buffer::create(state.device,
                                           {
                                               .size       = sz,
                                               .usage      = E_BufferUsage::TransferSrc,
                                               .memoryMode = E_MemoryMode::CpuToGpu,
                                           });
            stageBuf->fillBuffer(vertices.data(), sz, vertices.size());

            vBuf = Buffer::create(state.device,
                                  {
                                      .size       = sz,
                                      .usage      = E_BufferUsage::TransferDst | E_BufferUsage::Vertex,
                                      .memoryMode = E_MemoryMode::GpuOnly,
                                  });
            vBuf->copyFromBuffer(stageBuf, sz);
        }

        // ################## Index Buffer ################## //

        {
            VkDeviceSize sz = sizeof(indices[0]) * indices.size();
            auto stageBuf   = Buffer::create(state.device,
                                           {
                                               .size       = sz,
                                               .usage      = E_BufferUsage::TransferSrc,
                                               .memoryMode = E_MemoryMode::CpuToGpu,
                                           });
            stageBuf->fillBuffer(indices.data(), sz, indices.size());

            iBuf = Buffer::create(state.device,
                                  {
                                      .size       = sz,
                                      .usage      = E_BufferUsage::TransferDst | E_BufferUsage::Index,
                                      .memoryMode = E_MemoryMode::GpuOnly,
                                  });
            iBuf->copyFromBuffer(stageBuf, sz);
        }

        // ############### Create/Return Ptr ################ //

        return Ptr(new StaticMesh(vBuf, iBuf, pipelineIndex));
    }

    uint32_t StaticMesh::getPipelineIndex() const
    {
        return m_pipelineIndex;
    }

    ///////////////////////////////////////////////////////////////////////
    //                               Model                               //
    ///////////////////////////////////////////////////////////////////////

    StaticModel* StaticModel::loadImpl(const fs::path& p)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(
            p.string(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw std::runtime_error(utils::makeErrorMessage("ASSIMP", importer.GetErrorString()));
        }
        StaticModel* model = new StaticModel();
        model->processNode(scene->mRootNode, scene);
        return model;
    }

    void StaticModel::processNode(aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    StaticModel::mesh_ptr_t StaticModel::processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<vertex_t> vertices;
        std::vector<uint32_t> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertex_t vertex;
            vertex.position.x = mesh->mVertices[i].x;
            vertex.position.y = mesh->mVertices[i].y;
            vertex.position.z = mesh->mVertices[i].z;

            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;

            vertex.tangent.x = mesh->mTangents[i].x;
            vertex.tangent.y = mesh->mTangents[i].y;
            vertex.tangent.z = mesh->mTangents[i].z;

            if (mesh->mTextureCoords[0])
            {
                vertex.texCoords0.x = mesh->mTextureCoords[0][i].x;
                vertex.texCoords0.y = mesh->mTextureCoords[0][i].y;
            }
            else
            {
                vertex.texCoords0 = glm::vec2(0, 0);
            }
            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        uint32_t pipelineIndex = mesh->mMaterialIndex;

        return StaticMesh::create(vertices, indices, pipelineIndex);
    }
} // namespace ivulk
