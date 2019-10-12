#ifndef GAMEBOY_ADDRESS_H
#define GAMEBOY_ADDRESS_H

#include <type_traits>
#include <cpu/register16.h>
#include <memory/addressfwd.h>

namespace gameboy {

template<typename T>
class address {
    static_assert(std::is_unsigned_v<T>);

public:
    using size_type = T;

    constexpr address() = default;
    constexpr explicit address(size_type value)
        : value_(value) {}

    [[nodiscard]] constexpr size_type get_value() const { return value_; }

private:
    size_type value_ = 0x0u;
};

/**
 * Makes an address object
 * @param address an address value
 * @return An address object
 */
template<typename T>
constexpr address<T> make_address(T addr)
{
    return address<T>(addr);
}

/**
 * Makes an address object using a 16-bit register
 * @param reg a 16-bit register
 * @return An address object
 */
inline address16 make_address(const register16& reg)
{
    return address16(reg.value());
}

}

#endif //GAMEBOY_ADDRESS_H
