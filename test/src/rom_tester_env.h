#ifndef GAMEBOY_ROM_TESTER_ENV_H
#define GAMEBOY_ROM_TESTER_ENV_H

#include <filesystem>
#include <gtest/gtest.h>

class rom_tester_env : public testing::Environment {
public:
    explicit rom_tester_env(const char* res_base_path) noexcept
    {
        res_base_path_ = res_base_path;
    }

    static std::filesystem::path get_base_path() noexcept
    {
        return std::filesystem::path{res_base_path_};
    }

private:
    static const char* res_base_path_;
};

#endif //GAMEBOY_ROM_TESTER_ENV_H
