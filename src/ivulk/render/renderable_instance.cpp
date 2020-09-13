#include <ivulk/render/renderable_instance.hpp>

namespace ivulk {
	void RenderableInstance::render(std::weak_ptr<CommandBuffers> cmdBufs,
			glm::mat4 modelMatrix)
	{
		if (!pipeline.expired())
		{
			if (auto c = cmdBufs.lock())
			{
				c->bindPipeline(pipeline);
			}
		}
		modelMatrix = modelMatrix * transform.modelMatrix();
		if (auto r = renderable.lock())
		{
			r->render(cmdBufs, modelMatrix);
		}
	}
} // namespace ivulk
