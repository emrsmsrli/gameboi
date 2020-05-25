# Other CMake Flags

#### WITH_DEBUGGER:BOOL 

Enables debugger project to be built.

#### WITH_LIBCXX:BOOL: 

Use `libc++` instead of `libstc++`. 
Use this if linking with Clang gives errors.

#### ENABLE_TESTING:BOOL: 

Enables test project to be built. 
gameboi uses blargg's and mooneye's test roms to verify correctness of the emulation.
Currently most tests fail but games mostly work. \
You can run tests with `ctest` after building. \
You can also add more tests to the path `gameboycore/test/executable/path/res` if you want.

#### ENABLE_PCH:BOOL: 

Enables precompiled headers and puts commonly used STL headers into it.

#### ENABLE_IPO:BOOL: 

Enables link-time optimization. 
Can be used for additional optimization opportunities.

#### ENABLE_COVERAGE:BOOL: 

Enables coverage output. Only works for GCC and Clang compilers.

#### ENABLE_DOXYGEN:BOOL: 

Enables documentation output using Doxygen. 
Doxygen must be installed on your system. 
Currently outputs nothing.

#### ENABLE_CPPCHECK:BOOL: 

Enables cppcheck analyzer. cppcheck must be installed on your system. 
All warnings and errors are enabled by default.

#### ENABLE_CLANG_TIDY:BOOL: 

Enables clang-tidy analyzer. 
Clang-tidy must be installed on your system. 
Uses `.clang-tidy` file on the root of the project.

## SANITIZERS: 

ENABLE_SANITIZER_ADDRESS:BOOL \
ENABLE_SANITIZER_MEMORY:BOOL \
ENABLE_SANITIZER_UNDEFINED_BEHAVIOR:BOOL: