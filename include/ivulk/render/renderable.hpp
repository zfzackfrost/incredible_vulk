/**
 * @file renderable.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Define `I_Renderable` interface.
 */

#pragma once

#include <ivulk/glm.hpp>
#include <ivulk/render/priorities.hpp>
#include <memory>

namespace ivulk {
    class CommandBuffers;

    /**
	 * @brief Interface for objects that can be rendered to command buffers.
	 */
    class I_Renderable : public std::enable_shared_from_this<I_Renderable>
    {
    public:
        /**
		 * @brief Render this object to the given command buffers.
		 * 
		 * @param cmdBufs The set of command buffers to render to.
		 */
        virtual void render(std::weak_ptr<CommandBuffers> cmdBufs, glm::mat4 modelMatrix = glm::mat4(1)) = 0;

        /**
		 * @brief Get the priority for rendering this object.
		 *
		 * @return The priority for determining the order in which to render this object.
		 *         Higher priority renders sooner. Default implementation returns `0`.
		 */
        inline virtual int16_t renderOrder() const { return E_RenderPriority::Normal; }
    };
} // namespace ivulk
