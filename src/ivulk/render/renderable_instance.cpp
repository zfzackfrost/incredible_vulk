#include <ivulk/render/renderable_instance.hpp>
#include <ivulk/render/standard_shader.hpp>

namespace ivulk {
    void RenderableInstance::render(std::weak_ptr<CommandBuffers> cmdBufs, glm::mat4 modelMatrix)
    {
        if (!pipeline.expired())
        {
            if (auto c = cmdBufs.lock())
            {
                c->bindPipeline(pipeline);
            }
        }
        modelMatrix = modelMatrix * transform.modelMatrix();
        MatricesPushConstants matrices {
            .model = modelMatrix
        };
        if (auto r = renderable.lock())
        {
            if (auto c = cmdBufs.lock())
            {
                auto p = pipeline.lock();
                auto layout = p->getPipelineLayout();
                c->pushConstants(&matrices, layout, _size = sizeof(MatricesPushConstants));
            }
            r->render(cmdBufs, modelMatrix);
        }
    }
} // namespace ivulk
