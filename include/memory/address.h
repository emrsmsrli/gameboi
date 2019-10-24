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

    constexpr address() noexcept = default;
    constexpr explicit address(size_type value) noexcept
        : value_(value) {}

    [[nodiscard]] constexpr size_type value() const noexcept { return value_; }

private:
    size_type value_ = 0x0u;
};

template<typename T>
bool operator==(const address<T>& a_l, const address<T>& a_r) noexcept {
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

} // namespace gameboy

#endif //GAMEBOY_ADDRESS_H
