#include <filesystem>
#include <iostream>
#include <chrono>

#include <gtest/gtest.h>
#include "gameboy/gameboy.h"

namespace fs = std::filesystem;

namespace
{

class test_rom_runner {
public:
    explicit test_rom_runner(std::string path)
        : rom_path_{std::move(path)},
          gb_{rom_path_} {}

    uint8_t on_link_transfer(const uint8_t data) noexcept
    {
        link_buffer_ += static_cast<char>(data);

        if(link_buffer_.find("Pass") != std::string::npos) {
            test_completed_ = true;
            test_result_ = true;
        } else if(link_buffer_.find("Fail") != std::string::npos) {
            test_completed_ = true;
            test_result_ = false;
        }
        return 0xFFu;
    }

    void mock_render_line(uint8_t, const gameboy::render_line&) noexcept {}
    void mock_on_vblank() noexcept {}

    bool run() {
        using namespace std::chrono;

        constexpr auto timeout = 20s;
        const auto start = steady_clock::now();

        gb_.on_link_transfer_master({gameboy::connect_arg<&test_rom_runner::on_link_transfer>, this});
        gb_.on_render_line({gameboy::connect_arg<&test_rom_runner::mock_render_line>, this});
        gb_.on_vblank({gameboy::connect_arg<&test_rom_runner::mock_on_vblank>, this});

        while(!test_completed_) {
            gb_.tick_one_frame();

            if(steady_clock::now() - start > timeout) {
                std::cout << "test for " << rom_path_ << " timed out.\n";
                return false;
            }
        }

        if(!test_result_) {
            std::cout << link_buffer_ << '\n';
        }

        return test_result_;
    }

private:
    std::string rom_path_;
    gameboy::gameboy gb_;

    std::string link_buffer_;
    bool test_completed_ = false;
    bool test_result_ = false;
};

void do_run_test(const fs::path& path)
{
    for(const auto& file : fs::directory_iterator{path}) {
        std::cout << "running test rom at " << file.path() << '\n';

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
