/**
 * @file scene.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Define `Scene` class and related.
 */

#pragma once

#include <ivulk/render/renderable.hpp>

#include <set>

namespace ivulk {
    class Scene : public I_Renderable
    {
    public:
        using Ptr = std::shared_ptr<Scene>;
        using Ref = std::weak_ptr<Scene>;

        template <typename R>
        std::weak_ptr<R> addRenderable(const std::shared_ptr<R> rndbl)
        {
            static_assert(std::is_base_of_v<I_Renderable, R>,
                          "Template parameter `R` must derive from interface `I_Renderable`");
            std::shared_ptr<I_Renderable> r = std::static_pointer_cast<I_Renderable>(rndbl);
            m_renderables.insert(r);
            return rndbl;
        }

        template <typename R>
        void removeRenderable(const std::shared_ptr<R> rndbl)
        {
            static_assert(std::is_base_of_v<I_Renderable, R>,
                          "Template parameter `R` must derive from interface `I_Renderable`");
            std::shared_ptr<I_Renderable> r = std::static_pointer_cast<I_Renderable>(rndbl);
            m_renderables.erase(r);
        }

        virtual void render(std::weak_ptr<CommandBuffers> cmdBufs,
                            glm::mat4 modelMatrix = glm::mat4(1)) override;

        virtual inline int16_t renderOrder() const override { return E_RenderPriority::Normal - 100; }

        static inline Ptr create() { return Ptr(new Scene()); }

    private:
        Scene() = default;

        struct CompareRenderablePtr
        {
            inline bool operator()(const std::shared_ptr<I_Renderable>& a,
                                   const std::shared_ptr<I_Renderable>& b) const
            {
                return a->renderOrder() < b->renderOrder();
            }
        };
        std::multiset<std::shared_ptr<I_Renderable>, CompareRenderablePtr> m_renderables;
    };
} // namespace ivulk
