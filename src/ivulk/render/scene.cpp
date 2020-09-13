#include <algorithm>
#include <ivulk/core/command_buffer.hpp>
#include <ivulk/render/scene.hpp>

namespace ivulk {

    void Scene::render(std::weak_ptr<CommandBuffers> cmdBufs, glm::mat4 modelMatrix)
    {
        for (const auto& rndbl : m_renderables)
        {
            rndbl->render(cmdBufs, modelMatrix);
        }
    }
} // namespace ivulk
