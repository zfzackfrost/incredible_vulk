/**
 * @file renderable_instance.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `RenderableInstance` struct.
 */

#pragma once

#include <ivulk/render/renderable.hpp>
#include <ivulk/render/transform.hpp>
#include <ivulk/utils/keywords.hpp>

#include <ivulk/core/command_buffer.hpp>
#include <ivulk/core/graphics_pipeline.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/parameter.hpp>

#include <boost/mp11.hpp>

#include <functional>
#include <memory>
#include <optional>
#include <type_traits>

namespace ivulk {

    namespace detail { } // namespace detail

    /**
	 * @brief An instance of a renderable resource.
	 */
    class RenderableInstance : public I_Renderable
    {
    public:
        using Ptr = std::shared_ptr<RenderableInstance>;
        using Ref = std::weak_ptr<RenderableInstance>;

        virtual void render(std::weak_ptr<CommandBuffers> cmdBufs,
                            glm::mat4 modelMatrix = glm::mat4(1),
                            const std::vector<std::weak_ptr<GraphicsPipeline>>& pipelines = {}) override;
        virtual inline int16_t renderOrder() const override { return priority(); }

        // clang-format off
		BOOST_PARAMETER_MEMBER_FUNCTION(
			(Ptr), static create, tag, 
			(required
				(base, *)
			)
			(optional 
				(xform,    (Transform), Transform{})
				(priority, (std::optional<int16_t>), std::optional<int16_t>{})
				(getPriority, (std::optional<std::function<int16_t()>>), std::optional<std::function<int16_t()>>{})
				(pipeline, *, GraphicsPipeline::Ref{})
				(pipelines, *, std::vector<GraphicsPipeline::Ref>{})
			)
		)
        // clang-format on
        {
            return createImpl(std::weak_ptr(base), xform, priority, getPriority, pipeline, pipelines);
        }

        /**
		 * @brief Weak reference to the base renderable resource
		 */
        std::weak_ptr<I_Renderable> renderable;

        /**
		 * @brief Graphics pipelines to render with
		 */
        std::vector<GraphicsPipeline::Ref> pipelines;

        /**
		 * @brief Additional model matrix transform of the instance
		 */
        Transform transform;

    private:
        /**
		 * @brief Function object to get rendering order priority
		 */
        std::function<int16_t()> priority;

        template <typename BaseRenderable>
        static Ptr createImpl(std::weak_ptr<BaseRenderable> base,
                              Transform xform,
                              std::optional<int16_t> priority,
                              std::optional<std::function<int16_t()>> getPriority,
                              std::weak_ptr<GraphicsPipeline> pipeline,
                              const std::vector<GraphicsPipeline::Ref>& pipelines)
        {
            auto r = Ptr(new RenderableInstance());
            if (auto b = base.lock())
                r->renderable = b->weak_from_this();
            r->transform = xform;
            if (getPriority.has_value())
            {
                r->priority = *getPriority;
            }
            else if (priority.has_value())
            {
                r->priority = [priority]() -> int16_t { return *priority; };
            }
            r->pipelines = (pipelines.empty()) ? std::vector<GraphicsPipeline::Ref>{pipeline} : pipelines;
            return r;
        }
    };
} // namespace ivulk
