#ifndef GAMEBOY_ADDRESSRANGE_H
#define GAMEBOY_ADDRESSRANGE_H

#include <type_traits>
#include <memory/Address.h>

namespace gameboy::memory {

    /**
     * Represents a loopable address range in [low, high]
     */
    class AddressRange {
    public:
        class Iter {
        public:
            constexpr explicit Iter(uint16_t val) : value(val) {}
            [[nodiscard]] constexpr uint16_t operator*() const { return value; }
            [[nodiscard]] constexpr Iter operator++() const { return Iter(value + 1); }

        private:
            uint16_t value;
        };

        constexpr AddressRange(uint16_t begin, uint16_t end) :
                low(begin), high(end)
        {
            if(high < low) {
                std::swap(low, high);
            }
        }

        constexpr explicit AddressRange(const uint16_t& end) :
            AddressRange(0x0000u, end) {}
        constexpr explicit AddressRange(const Address16& end) :
            AddressRange(0x0000u, end.get_value()) {}
        constexpr AddressRange(const Address16& begin, const Address16& end) :
            AddressRange(begin.get_value(), end.get_value()) {}

        [[nodiscard]] constexpr uint16_t get_low() const { return low; }
        [[nodiscard]] constexpr uint16_t get_high() const { return high; }

        [[nodiscard]] constexpr bool contains(const Address16& address) const {
            return low <= address.get_value()
                && high >= address.get_value();
        }

    private:
        uint16_t low;
        uint16_t high;
    };

    constexpr AddressRange::Iter begin(const AddressRange& address_range) {
        return AddressRange::Iter(address_range.get_low());
    }

    constexpr AddressRange::Iter end(const AddressRange& address_range) {
        return AddressRange::Iter(address_range.get_high() + 1);
    }

    constexpr bool operator!=(const AddressRange::Iter& left, const AddressRange::Iter& right) {
        return *left != *right;
    }
}

#endif //GAMEBOY_ADDRESSRANGE_H
