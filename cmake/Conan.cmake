macro(run_conan)
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
        message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
        file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    conan_add_remote(NAME bincrafters URL https://api.bintray.com/conan/bincrafters/public-conan)
    conan_add_remote(NAME neargye URL https://api.bintray.com/conan/neargye/conan-packages)

    set(CONAN_LIBCXX "") # auto-detected
    if(WITH_LIBCXX)
        set(CONAN_LIBCXX compiler.libcxx=libc++)
    endif()

    conan_cmake_run(
            REQUIRES
              gtest/1.8.1@bincrafters/stable
              fmt/6.0.0@bincrafters/stable
              spdlog/1.4.2@bincrafters/stable
              sfml/2.5.1@bincrafters/stable
              magic_enum/0.6.4@neargye/stable
            OPTIONS
              sfml:graphics=True
              sfml:window=True
              sfml:audio=True
              sfml:network=False
            SETTINGS
              ${CONAN_LIBCXX}
            BASIC_SETUP
            CMAKE_TARGETS
            BUILD missing)
endmacro()
