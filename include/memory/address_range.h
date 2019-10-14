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
        constexpr explicit iterator(const uint16_t value)
            : value_(value) {}

        [[nodiscard]] constexpr uint16_t operator*() const { return value_; }
        [[nodiscard]] constexpr iterator operator++() const { return iterator(value_ + 1); }

    private:
        uint16_t value_;
    };

    constexpr address_range(const uint16_t begin, const uint16_t end)
        : begin_(begin), end_(end)
    {
        if(end_ < begin_) {
            std::swap(begin_, end_);
        }
    }

    constexpr explicit address_range(const uint16_t& end)
        : address_range(0x0000u, end) {}

    constexpr explicit address_range(const address16& end)
        : address_range(0x0000u, end.value()) {}

    constexpr address_range(const address16& begin, const address16& end)
        : address_range(begin.value(), end.value()) {}

    [[nodiscard]] constexpr uint16_t begin() const { return begin_; }
    [[nodiscard]] constexpr uint16_t end() const { return end_; }

    [[nodiscard]] constexpr bool contains(const address16& address) const
    {
        return begin_ <= address.value()
            && end_ >= address.value();
    }

private:
    uint16_t begin_;
    uint16_t end_;
};

constexpr address_range::iterator begin(const address_range& address_range)
{
    return address_range::iterator(address_range.begin());
}

constexpr address_range::iterator end(const address_range& address_range)
{
    return address_range::iterator(address_range.end() + 1);
}

constexpr bool operator!=(const address_range::iterator& left, const address_range::iterator& right)
{
    return *left != *right;
}

}

#endif //GAMEBOY_ADDRESS_RANGE_H
