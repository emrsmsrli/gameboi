#ifndef GAMEBOY_ADDRESS_RANGE_H
#define GAMEBOY_ADDRESS_RANGE_H

#include <type_traits>

#include <memory/address.h>

namespace gameboy {

/**
 * Represents a loopable address range in [low, high]
 */
class address_range {
public:
    class iterator {
    public:
        constexpr explicit iterator(const uint16_t value) noexcept
            : value_(value) {}

        [[nodiscard]] constexpr uint16_t operator*() const noexcept { return value_; }
        [[nodiscard]] constexpr iterator operator++() const noexcept { return iterator(value_ + 1); }

    private:
        uint16_t value_;
    };

    constexpr address_range(const uint16_t begin, const uint16_t end) noexcept
        : low_(begin), high_(end)
    {
        if(high_ < low_) {
            std::swap(low_, high_);
        }
    }

    constexpr explicit address_range(const uint16_t& end) noexcept
        : address_range(0x0000u, end) {}

    constexpr explicit address_range(const address16& end) noexcept
        : address_range(0x0000u, end.value()) {}

    constexpr address_range(const address16& begin, const address16& end) noexcept
        : address_range(begin.value(), end.value()) {}

    [[nodiscard]] constexpr uint16_t low() const noexcept { return low_; }
    [[nodiscard]] constexpr uint16_t high() const noexcept { return high_; }

    [[nodiscard]] constexpr bool contains(const address16& address) const noexcept
    {
        return low_ <= address.value()
            && high_ >= address.value();
    }

private:
    uint16_t low_;
    uint16_t high_;
};

constexpr address_range::iterator begin(const address_range& address_range) noexcept
{
    return address_range::iterator(address_range.low());
}

constexpr address_range::iterator end(const address_range& address_range) noexcept
{
    return address_range::iterator(address_range.high() + 1);
}

constexpr bool operator!=(const address_range::iterator& left, const address_range::iterator& right) noexcept
{
    return *left != *right;
}

}

#endif //GAMEBOY_ADDRESS_RANGE_H
