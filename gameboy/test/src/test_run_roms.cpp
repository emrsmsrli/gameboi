#include <filesystem>
#include <iostream>

#include "gtest/gtest.h"
#include "gameboy/gameboy.h"

namespace fs = std::filesystem;

namespace
{

class test_rom_runner {
public:
    explicit test_rom_runner(const std::string& path) : gb_{path} {}

    bool run() noexcept {
        std::string link_buffer;
        bool test_completed = true;
        bool test_result = false;

        // todo check link data for "Passed" or "Fail"

        while(!test_completed) {
            gb_.tick_one_frame();
        }

        if(!test_result) {
            std::cout << link_buffer << '\n';
        }

        return test_result;
    }

private:
    gameboy::gameboy gb_;
};

void do_run_test(const fs::path& path)
{
    for(const auto& file : fs::directory_iterator{path}) {
        std::cout << "running rom at " << file.path() << '\n';

        test_rom_runner runner{file.path().string()};
        ASSERT_TRUE(runner.run());
    }
}

} // namespace

TEST(run_roms, test_cpu_instrs) {
    do_run_test(fs::current_path().append("res").append("cpu_instrs"));
}

TEST(run_roms, test_cgb_sound) {
    do_run_test(fs::current_path().append("res").append("cgb_sound"));
}

TEST(run_roms, test_dmg_sound) {
    do_run_test(fs::current_path().append("res").append("dmg_sound"));
}

TEST(run_roms, test_mem_timing) {
    do_run_test(fs::current_path().append("res").append("mem_timing"));
}

TEST(run_roms, test_mem_timing_2) {
    do_run_test(fs::current_path().append("res").append("mem_timing_2"));
}

TEST(run_roms, test_oam_bug) {
    do_run_test(fs::current_path().append("res").append("oam_bug"));
}

TEST(run_roms, test_general) {
    do_run_test(fs::current_path().append("res"));
}
