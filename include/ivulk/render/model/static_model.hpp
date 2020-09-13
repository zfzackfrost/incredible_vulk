/**
 * @file static_model.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `StaticModel` and related types.
 */

#pragma once

#include <ivulk/core/vertex.hpp>

#include <assimp/scene.h>

#include <ivulk/render/model/base.hpp>

namespace ivulk {

    // clang-format off
	IVULK_VERTEX_STRUCT(StaticMeshVertex, 
		((glm::vec3, position, 0))
		((glm::vec3, normal, 1))
		((glm::vec3, tangent, 2))
		((glm::vec2, texCoords0, 3))
	);
    // clang-format on

    class StaticMesh final
    {
    public:
        using Ptr = std::shared_ptr<StaticMesh>;
        using Ref = std::weak_ptr<StaticMesh>;

        using vertex_t = StaticMeshVertex;

        StaticMesh() = delete;

        static Ptr create(const std::vector<vertex_t>& vertices, const std::vector<uint32_t>& indices, uint32_t pipelineIndex);

        uint32_t getPipelineIndex() const;

        Buffer::Ref getIndexBuffer() const { return m_indexBuffer; }
        Buffer::Ref getVertexBuffer() const { return m_vertexBuffer; }

    private:
        StaticMesh(Buffer::Ptr vBuf, Buffer::Ptr iBuf, uint32_t pipelineIndex);

        Buffer::Ptr m_vertexBuffer;
        Buffer::Ptr m_indexBuffer;
        uint32_t m_pipelineIndex;
    };

    class StaticModel final : public ModelBase<StaticModel, StaticMesh>
    {
    public:
    private:
        friend model_base_t;

        void processNode(aiNode* node, const aiScene* scene);
        mesh_ptr_t processMesh(aiMesh* mesh, const aiScene* scene);

        static StaticModel* loadImpl(const boost::filesystem::path& p);
    };
} // namespace ivulk
