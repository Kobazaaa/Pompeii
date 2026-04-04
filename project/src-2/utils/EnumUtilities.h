#ifndef POMPEII_ENUM_UTILITIES_H
#define POMPEII_ENUM_UTILITIES_H

// -- Standard Library --
#include <type_traits>

#define ENUM_AS_FLAGS(EnumType)                                                                     \
                                                                                                    \
    inline EnumType operator|(EnumType lhs, EnumType rhs)                                           \
    {                                                                                               \
        using UnderlyingType = std::underlying_type_t<EnumType>;                                    \
        return static_cast<EnumType>(                                                               \
                    static_cast<UnderlyingType>(lhs) |                                              \
                    static_cast<UnderlyingType>(rhs)                                                \
                );                                                                                  \
    }                                                                                               \
    inline EnumType& operator|=(EnumType& lhs, EnumType rhs)                                        \
    {                                                                                               \
        return lhs = lhs | rhs;                                                                     \
    }                                                                                               \
                                                                                                    \
    inline EnumType operator&(EnumType lhs, EnumType rhs)                                           \
    {                                                                                               \
        using UnderlyingType = std::underlying_type_t<EnumType>;                                    \
        return static_cast<EnumType>(                                                               \
                    static_cast<UnderlyingType>(lhs) &                                              \
                    static_cast<UnderlyingType>(rhs)                                                \
                );                                                                                  \
    }                                                                                               \
    inline EnumType& operator&=(EnumType& lhs, EnumType rhs)                                        \
    {                                                                                               \
        return lhs = lhs & rhs;                                                                     \
    }                                                                                               \
                                                                                                    \
    inline EnumType operator~(EnumType lhs)                                                         \
    {                                                                                               \
        using UnderlyingType = std::underlying_type_t<EnumType>;                                    \
        return static_cast<EnumType>(                                                               \
                    ~static_cast<UnderlyingType>(lhs)                                               \
                );                                                                                  \
    }                                                                                               \
                                                                                                    \
    inline bool HasAnyFlags(EnumType value, EnumType flag)                                          \
    {                                                                                               \
        using UnderlyingType = std::underlying_type_t<EnumType>;                                    \
        return (static_cast<UnderlyingType>(value) &                                                \
                static_cast<UnderlyingType>(flag)) != 0;                                            \
    }                                                                                               \

#endif // POMPEII_ENUM_UTILITIES_H
