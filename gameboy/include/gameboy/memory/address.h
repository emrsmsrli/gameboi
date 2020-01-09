#ifndef GAMEBOY_ADDRESS_H
#define GAMEBOY_ADDRESS_H

#include <type_traits>

#include "gameboy/cpu/register16.h"
#include "gameboy/memory/addressfwd.h"

namespace gameboy {

template<typename T>
class address {
    static_assert(std::is_unsigned_v<T>);

public:
    using size_type = T;

    constexpr address() noexcept = default;
    constexpr explicit address(const size_type value) noexcept
        : value_{value} {}

    [[nodiscard]] constexpr size_type value() const noexcept { return value_; }

    constexpr address<T> operator+(const T value) const noexcept { return address<T>(value_ + value); }
    constexpr address<T> operator-(const T value) const noexcept { return address<T>(value_ - value); }

    constexpr address<T>& operator=(const T value) noexcept
    {
        value_ = value;
        return *this;
    }

private:
    size_type value_ = 0x0u;
};

template<typename T>
constexpr bool operator==(const address<T>& a_l, const address<T>& a_r) noexcept
{
    return a_l.value() == a_r.value();
}

/**
 * Makes an address object
 * @param addr an address value
 * @return An address object
 */
template<typename T>
constexpr address<T> make_address(T addr) noexcept
{
    return address<T>(addr);
}

/**
 * Makes an address object using a 16-bit register
 * @param reg a 16-bit register
 * @return An address object
 */
inline address16 make_address(const register16& reg) noexcept
{
    return address16(reg.value());
}

constexpr size_t operator ""_kb(const unsigned long long bytes)
{
    return bytes * 1024u;
}

} // namespace gameboy

template<>
struct std::hash<gameboy::address16> {
    std::size_t operator()(const gameboy::address16& addr) const noexcept
    {
        return std::hash<uint16_t>{}(addr.value());
    }
};

#endif //GAMEBOY_ADDRESS_H
