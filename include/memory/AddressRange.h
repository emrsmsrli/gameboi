//
// Created by Emre Şimşirli on 26.08.2019.
//

#ifndef GAMEBOY_ADDRESSRANGE_H
#define GAMEBOY_ADDRESSRANGE_H

#include "memory/Address.h"

namespace gameboy::memory {

    /**
     * Represents a loopable address range in [low, high]
     */
    class AddressRange {
    public:
        class Iter {
        public:
            explicit Iter(uint16_t val) : value(val) {}
            [[nodiscard]] uint16_t operator*() const { return value; }
            [[nodiscard]] Iter operator++() const { return Iter(value + 1); }

        private:
            uint16_t value;
        };

        AddressRange(uint16_t begin, uint16_t end) :
                low(begin), high(end)
        {
            if(high < low) {
                std::swap(low, high);
            }
        }

        AddressRange(const Address16& begin, const Address16& end) :
            AddressRange(begin.get_value(), end.get_value()) {}

        [[nodiscard]] uint16_t get_low() const { return low; }
        [[nodiscard]] uint16_t get_high() const { return high; }

        [[nodiscard]] bool contains(const Address16& address) const {
            return low <= address.get_value()
                && high >= address.get_value();
        }

    private:
        uint16_t low;
        uint16_t high;
    };

    AddressRange::Iter begin(const AddressRange& address_range) {
        return AddressRange::Iter(address_range.get_low());
    }

    AddressRange::Iter end(const AddressRange& address_range) {
        return AddressRange::Iter(address_range.get_high() + 1);
    }

    bool operator!=(const AddressRange::Iter& left, const AddressRange::Iter& right) {
        return *left != *right;
    }
}

#endif //GAMEBOY_ADDRESSRANGE_H
