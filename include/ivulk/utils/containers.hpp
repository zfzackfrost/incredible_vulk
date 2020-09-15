/**
 * @file containers.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Container utility functions
 */

#pragma once

#include <ivulk/config.hpp>

#include <vector>

namespace ivulk::utils {
    template <typename To, typename From, typename... ContainerArgs>
    auto castVector(const std::vector<From, ContainerArgs...>& c)
    {
        return std::vector<To>(c.begin(), c.end());
    }
} // namespace ivulk::utils
