#define IVULK_SOURCE
#include <ivulk/config.hpp>

#include <algorithm>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/render/scene.hpp>

namespace ivulk {

    void Scene::render(std::weak_ptr<CommandBuffers> cmdBufs, glm::mat4 modelMatrix, const std::vector<std::weak_ptr<GraphicsPipeline>>& pipelines)
    {
        for (const auto& rndbl : m_renderables)
        {
            rndbl->render(cmdBufs, modelMatrix, pipelines);
        }
    }
} // namespace ivulk
