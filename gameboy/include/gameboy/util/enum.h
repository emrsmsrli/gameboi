#ifndef GAMEBOY_ENUM_H
#define GAMEBOY_ENUM_H

#include <type_traits>

namespace gameboy {

#define BITMASK(name) \
name operator|(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) | static_cast<std::underlying_type_t<name>>(r)); } \
name operator&(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) & static_cast<std::underlying_type_t<name>>(r)); } \
name operator^(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) ^ static_cast<std::underlying_type_t<name>>(r)); } \
name& operator|=(name& l, name r) { l = l | r; return l; } \
name& operator&=(name& l, name r) { l = l & r; return l; } \
name& operator^=(name& l, name r) { l = l ^ r; return l; } \
name operator~(name l) { return static_cast<name>(~static_cast<std::underlying_type_t<name>>(l)); } \

#define BITMASKF(name) \
friend name operator|(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) | static_cast<std::underlying_type_t<name>>(r)); } \
friend name operator&(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) & static_cast<std::underlying_type_t<name>>(r)); } \
friend name operator^(name l, name r) { return static_cast<name>(static_cast<std::underlying_type_t<name>>(l) ^ static_cast<std::underlying_type_t<name>>(r)); } \
friend name& operator|=(name& l, name r) { l = l | r; return l; } \
friend name& operator&=(name& l, name r) { l = l & r; return l; } \
friend name& operator^=(name& l, name r) { l = l ^ r; return l; } \
friend name operator~(name l) { return static_cast<name>(~static_cast<std::underlying_type_t<name>>(l)); } \

} // namespace gameboy

#endif //GAMEBOY_ENUM_H
