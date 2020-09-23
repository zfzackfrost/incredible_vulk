/**
 * @file base.hpp
 * @author Zachary Frost
 * @copyright MIT License (See LICENSE.md in repostory root)
 * @brief Base unit types.
 */

#pragma once

#define IVULK_UNIT_TYPE(UnitName)                                                                            \
    template <typename Ratio>                                                                                \
    class UnitName                                                                                           \
    {                                                                                                        \
    public:                                                                                                  \
        UnitName()                                                                                           \
            : m_count(0)                                                                                     \
        { }                                                                                                  \
        explicit UnitName(double c)                                                                          \
            : m_count(c)                                                                                     \
        { }                                                                                                  \
        double count() const { return m_count; }                                                             \
                                                                                                             \
        template <typename Ratio2>                                                                           \
        operator UnitName<Ratio2>() const                                                                    \
        {                                                                                                    \
            auto num1     = Ratio::num;                                                                      \
            auto den1     = Ratio::den;                                                                      \
            auto num2     = Ratio2::num;                                                                     \
            auto den2     = Ratio2::den;                                                                     \
            double scale1 = static_cast<double>(num1) / static_cast<double>(den1);                           \
            double scale2 = static_cast<double>(num2) / static_cast<double>(den2);                           \
            return UnitName<Ratio2>((count() * scale1) / scale2);                                            \
        }                                                                                                    \
        operator double() const { return count(); }                                                          \
                                                                                                             \
    private:                                                                                                 \
        double m_count;                                                                                      \
    };                                                                                                       \
    template <typename Ratio1, typename Ratio2>                                                              \
    UnitName<Ratio1> operator+(const UnitName<Ratio1>& a, const UnitName<Ratio2>& b)                         \
    {                                                                                                        \
        using Result = UnitName<Ratio1>;                                                                     \
        Result tmpA  = a;                                                                                    \
        Result tmpB  = b;                                                                                    \
        return Result(tmpA.count() + tmpB.count());                                                          \
    }                                                                                                        \
    template <typename Ratio1, typename Ratio2>                                                              \
    UnitName<Ratio1> operator-(const UnitName<Ratio1>& a, const UnitName<Ratio2>& b)                         \
    {                                                                                                        \
        using Result = UnitName<Ratio1>;                                                                     \
        Result tmpA  = a;                                                                                    \
        Result tmpB  = b;                                                                                    \
        return Result(tmpA.count() - tmpB.count());                                                          \
    }                                                                                                        \
    template <typename Ratio1, typename Ratio2>                                                              \
    UnitName<Ratio1> operator*(const UnitName<Ratio1>& a, const UnitName<Ratio2>& b)                         \
    {                                                                                                        \
        using Result = UnitName<Ratio1>;                                                                     \
        Result tmpA  = a;                                                                                    \
        Result tmpB  = b;                                                                                    \
        return Result(tmpA.count() * tmpB.count());                                                          \
    }                                                                                                        \
    template <typename Ratio1, typename Ratio2>                                                              \
    UnitName<Ratio1> operator/(const UnitName<Ratio1>& a, const UnitName<Ratio2>& b)                         \
    {                                                                                                        \
        using Result = UnitName<Ratio1>;                                                                     \
        Result tmpA  = a;                                                                                    \
        Result tmpB  = b;                                                                                    \
        return Result(tmpA.count() / tmpB.count());                                                          \
    }
