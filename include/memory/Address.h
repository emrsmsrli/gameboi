#ifndef GAMEBOY_ADDRESS_H
#define GAMEBOY_ADDRESS_H

#include <type_traits>
#include <cstdint>
#include "AddressMap.h"
#include "AddressFwd.h"

namespace gameboy::cpu {
    class Register8;
    class Register16;
}

namespace gameboy::memory {
    template<typename T>
    class Address {
        static_assert(std::is_unsigned_v<T>);

    public:
        using size_type = T;

        constexpr Address() = default;
        constexpr explicit Address(size_type default_value) : value(default_value) {}

        [[nodiscard]] constexpr size_type get_value() const { return value; }

    private:
        size_type value = 0x0u;
    };

    /**
     * Makes an address object
     * @param address an address value
     * @return An address object
     */
    template<typename T>
    Address<T> make_address(T address) {
        return Address<T>(address);
    }

    /**
     * Makes an address object
     * @param location two byte memory location
     * @return An address object
     */
    Address16 make_address(Map location);

    /**
     * Makes an address object using a 16-bit register
     * @param reg a 16-bit register
     * @return An address object
     */
    Address16 make_address(const cpu::Register16& reg);
}

#endif //GAMEBOY_ADDRESS_H
