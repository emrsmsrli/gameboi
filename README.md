# gameboi

[![Build Status](https://travis-ci.com/emrsmsrli/gameboi.svg?branch=master)](https://travis-ci.com/emrsmsrli/gameboi)
[![GitHub license](https://img.shields.io/github/license/Naereen/StrapDown.js.svg)](https://github.com/emrsmsrli/gameboi/blob/master/LICENSE)
[![Github Release](https://img.shields.io/github/v/release/emrsmsrli/gameboi)](https://github.com/emrsmsrli/gameboi/releases)
[![Code Coverage](https://img.shields.io/codecov/c/gh/emrsmsrli/gameboi)](https://codecov.io/gh/emrsmsrli/gameboi)

gameboi is a Gameboy Color emulator written in Modern C++. Main goals of this project are 
writing understandable, modern C++ and not sacrificing performance, 
and learning about software & hardware architectures while doing so. 
This emulator does not have the best accuracy, but things mostly work.

### Features

gameboi does not aim to be the perfect emulator, 
so features are limited compared to other emulators.

- CPU, PPU and APU emulation
- Accurate internal timer
- Cartridge save-load capability
- Cartridge RTC support
- Link support
- Debugger and disassembler

## Screenshots

<p align="center">
    <img src="screenshots/mario.png" height=240 />
    <img src="screenshots/shantae.png" height=240 />
</p>
<p align="center">
    <img src="screenshots/zelda.png" height=240 />
    <img src="screenshots/tetris.png" height=240 />
</p>
<p align="center">
    <img src="screenshots/debugger.png" width=750 />
</p>

## Code example

gameboi-core library can be easily integrated to your own frontend implementation:
```cpp
#include <gameboy/gameboy.h>

// automatically defined with cmake argument -DWITH_DEBUGGER=ON
#if WITH_DEBUGGER
#include <debugger/debugger.h>
#endif //WITH_DEBUGGER

void on_render_line(const uint8_t line_number, const gameboy::render_line& line) noexcept;
void on_vblank() noexcept;
void on_audio(const gameboy::apu::sound_buffer& sound_buffer) noexcept;

int main(int argc, char** argv) 
{
    // gameboy writes cartridge save files to disk 
    // when it goes out of scope
    gameboy::gameboy gb{"file/path/to/rom.gbc"};

#if WITH_DEBUGGER
    gameboy::debugger debugger{gb};
#endif //WITH_DEBUGGER

    gb.on_render_line(gameboy::connect_arg<&on_render_line>);
    gb.on_vblank(gameboy::connect_arg<&on_vblank>);
    gb.on_audio_buffer_full(gameboy::connect_arg<&on_audio>);

    while(true) {
        // gb.press_key(gameboy::key::a);
        // gb.release_key(gameboy::key::start);

        gb.tick_one_frame();
        // or gb.tick(); which executes only one instruction every iteration

#if WITH_DEBUGGER
        debugger.tick();
#endif //WITH_DEBUGGER
    }

    return 0;
}
```

## Build Instructions

### Dependencies

Install dependencies however you like. Though, `vcpkg` or `conan` might come in handy.
This is an example script for Ubuntu, using vcpkg:

- CMake (required version is 3.12.4)
- GTest (required if building tests)
- spdlog
- fmt
- SFML
- SDL2
- magic-enum
- cxxopts

```shell script
$ git clone https://github.com/Microsoft/vcpkg.git
$ cd vcpkg
$ ./bootstrap-vcpkg.sh -disableMetrics
$ ./vcpkg integrate install
$ ./vcpkg install
$ ./vcpkg install gtest spdlog fmt sfml sdl2 magic-enum cxxopts
$ sudo apt install cmake ninja-build
```

### Building from source

gameboi uses CMake and can be easily built with a script like below.

```shell script
$ mkdir build
$ cd build
$ cmake -G Ninja --config Release ..
$ cmake --build -- -j $(nproc)
```

### CMake arguments

gameboi offers several arguments for build configuration

**WITH_DEBUGGER:BOOL**: Enables debugger project to be built.

**WITH_LIBCXX:BOOL**: Use `libc++` instead of `libstc++`. 
Use this if linking with Clang gives errors.

**ENABLE_TESTING:BOOL**: Enables test project to be built. You can run tests with `ctest`.

**ENABLE_PCH:BOOL**: Enables precompiled headers and puts commonly used STL headers into it.

**ENABLE_IPO:BOOL**: Enables link-time optimization. 
Can be used for additional optimization opportunities.

**ENABLE_COVERAGE:BOOL**: Enables coverage output. Only works for GCC and Clang compilers.

**ENABLE_DOXYGEN:BOOL**: Enables documentation output using Doxygen. 
Doxygen must be installed on your system. 
Currently outputs nothing.

**ENABLE_SANITIZER_ADDRESS:BOOL**: \
**ENABLE_SANITIZER_MEMORY:BOOL**: \
**ENABLE_SANITIZER_UNDEFINED_BEHAVIOR:BOOL**: 
Enables the corresponding sanitizer. Only works for GCC and Clang compilers.

**ENABLE_CPPCHECK:BOOL**: Enables cppcheck analyzer. cppcheck must be installed on your system. 
All warnings and errors are enabled by default.

**ENABLE_CLANG_TIDY:BOOL**: Enables clang-tidy analyzer. 
Clang-tidy must be installed on your system. 
Uses `.clang-tidy` file on the root of the project.

## Known Issues

* Double speed breaks data transfer to PPU.
* Debugger currently depends on SFML.
