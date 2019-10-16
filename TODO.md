# TODO

- integer promotion is undefined behaviour, use casts
- try to organize `cpu::decode`
- implement templated `AddressRange`
- change barebones bit manipulation expressions with `math::bit_*`
- extend `gameboy::math` functionality
- make constexpr `AddressRange`s in a header file
- `MMU::set_interrupt_master_enable` might fail 
    because addr 0xffff might not be available. investigate
- interrupts should enable-disable after one instruction is executed.
- reduce branching please
- return pairs instead of passing out params
- use cpp11 uniform init syntax (constructor init list included)
- make code noexcept-correct

## Major
- refactor mbcs. (maybe use virtual-final combo for devirtualization)
