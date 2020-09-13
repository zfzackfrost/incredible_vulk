/**
 * @file scene.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Define `Scene` class and related.
 */

#pragma once

#include <ivulk/render/renderable.hpp>
#include <ivulk/render/renderable_instance.hpp>

#include <set>

namespace ivulk {
    class RenderableInstance;
    class Scene : public I_Renderable
    {
    public:
        using Ptr = std::shared_ptr<Scene>;
        using Ref = std::weak_ptr<Scene>;

        std::weak_ptr<RenderableInstance> addRenderable(const std::shared_ptr<RenderableInstance> rndbl)
        {
            m_renderables.insert(rndbl);
            return rndbl;
        }

        void removeRenderable(const std::shared_ptr<RenderableInstance> rndbl)
        {
            m_renderables.erase(rndbl);
        }

        virtual void render(std::weak_ptr<CommandBuffers> cmdBufs,
                            glm::mat4 modelMatrix = glm::mat4(1)) override;

        virtual inline int16_t renderOrder() const override { return E_RenderPriority::Normal - 100; }

        static inline Ptr create() { return Ptr(new Scene()); }

    private:
        Scene() = default;

        struct CompareRenderablePtr
        {
            inline bool operator()(const std::shared_ptr<RenderableInstance>& a,
                                   const std::shared_ptr<RenderableInstance>& b) const
            {
                return a->renderOrder() < b->renderOrder(); 
            }
        };
        std::multiset<std::shared_ptr<RenderableInstance>, CompareRenderablePtr> m_renderables;
    };
} // namespace ivulk
