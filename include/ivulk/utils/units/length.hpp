/**
 * @file length.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Types for compile-time length units.
 */

#pragma once

#include <ivulk/glm.hpp>
#include <ivulk/utils/units/base.hpp>

#include <ratio>

namespace ivulk {
    IVULK_UNIT_TYPE(UnitMeter);

    // Metric
    using meter      = UnitMeter<std::ratio<1, 1>>;
    using millimeter = UnitMeter<std::milli>;
    using centimeter = UnitMeter<std::centi>;
    using kilometer  = UnitMeter<std::kilo>;

    // Imperial
    using foot = UnitMeter<std::ratio<381, 1250>>;
    using inch = UnitMeter<std::ratio<127, 5000>>;
    using mile = UnitMeter<std::ratio<201168, 125>>;
    using yard = UnitMeter<std::ratio<1143, 1250>>;

    struct Position
    {
        meter x = meter {0};
        meter y = meter {0};
        meter z = meter {0};

        [[nodiscard]] glm::vec3 toVec() const
        {
            return {
                x.count(),
                y.count(),
                z.count(),
            };
        }
    };

    inline Position operator+(const Position& a, const Position& b)
    {
        return {
            .x = a.x + b.x,
            .y = a.y + b.y,
            .z = a.z + b.z,
        };
    }
    inline Position operator-(const Position& a, const Position& b)
    {
        return {
            .x = a.x - b.x,
            .y = a.y - b.y,
            .z = a.z - b.z,
        };
    }
    inline Position operator*(const Position& a, const Position& b)
    {
        return {
            .x = a.x * b.x,
            .y = a.y * b.y,
            .z = a.z * b.z,
        };
    }
    inline Position operator/(const Position& a, const Position& b)
    {
        return {
            .x = a.x / b.x,
            .y = a.y / b.y,
            .z = a.z / b.z,
        };
    }

    inline Position operator+(const Position& a, const meter& b)
    {
        return {
            .x = a.x + b,
            .y = a.y + b,
            .z = a.z + b,
        };
    }
    inline Position operator-(const Position& a, const meter& b)
    {
        return {
            .x = a.x - b,
            .y = a.y - b,
            .z = a.z - b,
        };
    }
    inline Position operator*(const Position& a, const meter& b)
    {
        return {
            .x = a.x * b,
            .y = a.y * b,
            .z = a.z * b,
        };
    }
    inline Position operator/(const Position& a, const meter& b)
    {
        return {
            .x = a.x / b,
            .y = a.y / b,
            .z = a.z / b,
        };
    }
} // namespace ivulk
