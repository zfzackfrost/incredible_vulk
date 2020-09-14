/**
 * @file renderer.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief `Renderer` class and related.
 */

#pragma once

#include <ivulk/config.hpp>

#include <ivulk/core/app_state.hpp>

namespace ivulk {
    class App;

    class Renderer
    {
    public:
        explicit Renderer(App* ownerApp);

        static Renderer* current();

        void drawFrame();

    protected:
        AppState& state;
        static void makeCurrent(Renderer* newCurrent = nullptr);

        uint32_t m_currentFrame;
    };
} // namespace ivulk
