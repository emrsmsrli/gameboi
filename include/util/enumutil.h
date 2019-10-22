#ifndef GAMEBOY_ENUMUTIL_H
#define GAMEBOY_ENUMUTIL_H

#include <type_traits>

namespace gameboy {

template<typename>
struct enable_bitmask_operators {
    static constexpr bool value = false;
};

template<typename E>
inline constexpr bool enable_bitmask_operators_v = enable_bitmask_operators<E>::value;

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E>
operator&(E lhs, E rhs)
{
    using underlying_type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying_type>(lhs) & static_cast<underlying_type>(rhs));
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E>
operator|(E lhs, E rhs)
{
    using underlying_type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying_type>(lhs) | static_cast<underlying_type>(rhs));
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E>
operator^(E lhs, E rhs)
{
    using underlying_type = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying_type>(lhs) ^ static_cast<underlying_type>(rhs));
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E>
operator~(E e)
{
    using underlying_type = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<underlying_type>(e));
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E&>
operator&=(E& lhs, E rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E&>
operator|=(E& lhs, E rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<typename E>
std::enable_if_t<enable_bitmask_operators_v<E>, E&>
operator^=(E& lhs, E rhs)
{
    lhs = lhs ^ rhs;
    return lhs;
}

}

#define DEFINE_ENUM_CLASS_FLAGS(E) \
template<> \
struct enable_bitmask_operators<E> { \
    static constexpr bool value = true; \
} \

#endif //GAMEBOY_ENUMUTIL_H
