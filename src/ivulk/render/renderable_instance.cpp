#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <ivulk/render/renderable_instance.hpp>
#include <ivulk/render/standard_shader.hpp>

namespace ivulk {
    void RenderableInstance::render(std::weak_ptr<CommandBuffers> cmdBufs, glm::mat4 modelMatrix, const std::vector<std::weak_ptr<GraphicsPipeline>>& pipelines)

    {
        modelMatrix = modelMatrix * transform.modelMatrix();
        if (auto r = renderable.lock())
        {
            r->render(cmdBufs, modelMatrix, this->pipelines);
        }
    }
} // namespace ivulk
