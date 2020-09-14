/**
 * @file base.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `ModelBase` class
 */

#pragma once

#include <ivulk/config.hpp>

#include <boost/filesystem.hpp>
#include <ivulk/core/buffer.hpp>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/render/renderable.hpp>
#include <ivulk/render/standard_shader.hpp>

#include <ivulk/utils/fs.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace ivulk {

    template <typename Derived, typename Mesh>
    class ModelBase : public I_Renderable
    {
    public:
        using Ptr = std::shared_ptr<Derived>;
        using Ref = std::weak_ptr<Derived>;

        using mesh_ptr_t   = typename Mesh::Ptr;
        using mesh_ref_t   = typename Mesh::Ref;
        using vertex_t     = typename Mesh::vertex_t;
        using model_base_t = ModelBase<Derived, Mesh>;

        virtual void render(std::weak_ptr<CommandBuffers> cmdBufs,
                            glm::mat4 modelMatrix = glm::mat4(1),
                            const std::vector<GraphicsPipeline::Ref>& pipelines = {}) override
        {

            using namespace tag;
            MatricesPushConstants matrices {
                .model = modelMatrix
            };

            if (auto c = cmdBufs.lock())
            {
                for (const auto& m : meshes)
                {
                    uint32_t pipelineIndex = m->getPipelineIndex();
                    if (pipelineIndex < pipelines.size())
                    {
                        auto pipeline = pipelines[pipelineIndex];

                        auto p = pipeline.lock();
                        auto layout = p->getPipelineLayout();
                        c->bindPipeline(pipeline);
                        c->pushConstants(&matrices, layout, _size = sizeof(MatricesPushConstants));
                    }
                    Buffer::Ref vBuf = m->getVertexBuffer();
                    Buffer::Ref iBuf = m->getIndexBuffer();
                    c->draw(_vertexBuffer = vBuf, _indexBuffer = iBuf);
                }
            }
        }

        static Ptr load(const boost::filesystem::path& p)
        {
            static_assert(
                std::is_same_v<decltype(Derived::loadImpl(boost::filesystem::path {})), Derived*>,
                "Invalid signiture for `ModelBase` subclass's implementation of static method `loadImpl`");

            // ############### Prepare file path ################ //

            auto loadPath = utils::prepareAssetPath(p);
            if (!loadPath.has_value())
            {
                throw std::runtime_error(
                    utils::makeErrorMessage("FILE", "Invalid or missing model file path"));
            }

            return Ptr(Derived::loadImpl(*loadPath));
        }

    protected:
        std::vector<mesh_ptr_t> meshes;
    };
} // namespace ivulk
