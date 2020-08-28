#ifndef GAMEBOY_FRONTEND_H
#define GAMEBOY_FRONTEND_H

#include <optional>
#include <cstdint>

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <nlohmann/json.hpp>

#include "gameboy/gameboy.h"
#include "sdl_audio.h"

class frontend {
public:
    enum class tick_result { ticking, paused, should_exit };

    using on_new_rom_func = gameboy::delegate<void()>;

    frontend(uint32_t width, uint32_t height, bool fullscreen, const gameboy::filesystem::path& rom_base_path) noexcept;
    frontend(uint32_t width, uint32_t height, bool fullscreen) noexcept;
    ~frontend();

    frontend(const frontend&) = delete;
    frontend(frontend&&) = delete;

    frontend& operator=(const frontend&) = delete;
    frontend& operator=(frontend&&) = delete;

    sf::RenderWindow& window() noexcept { return window_; }
    const sf::RenderWindow& window() const noexcept { return window_; }

    void on_new_rom(const on_new_rom_func on_new_rom) noexcept { on_new_rom_ = on_new_rom; }

    /** @return true if should continue ticking, false otherwise */
    [[nodiscard]] tick_result tick();

    void play_sound(const gameboy::apu::sound_buffer& sound_buffer) noexcept;
    void rescale_view() noexcept;
    void render_line(uint8_t line_number, const gameboy::render_line& line) noexcept;
    void draw_sprite() noexcept;
    void render_frame() noexcept;

    void register_gameboy(gameboy::observer<gameboy::gameboy> gb) noexcept;

private:
    struct rom {
        gameboy::filesystem::path path;
        std::optional<uint32_t> gb_palette_idx;
        bool is_favorite;
    };

    enum class state {
        game,
        main_menu,
        select_audio_device,
        select_gb_color_palette,
        select_rom_file
    };

    nlohmann::json config_;
    std::vector<rom> roms_;

    gameboy::observer<gameboy::gameboy> gb_;
    sf::Image window_buffer_;
    sf::Texture window_texture_;
    sf::Sprite window_sprite_;
    sf::RenderWindow window_;

    sf::Event event_{};
    sf::Clock dt_;

    sdl::audio_device audio_device_;

    state state_{state::select_rom_file};
    int32_t menu_selected_index_ = -1;
    int32_t menu_max_index_ = -1;

    on_new_rom_func on_new_rom_;

    void handle_game_keys(const sf::Event& key_event) noexcept;

    void draw_menu() noexcept;
    void draw_audio_device_select() noexcept;
    void draw_gb_palette_select() noexcept;
    void draw_rom_select() noexcept;

    [[nodiscard]] bool can_pick_gb_color_palette() noexcept
    {
        return gb_ &&
          !gb_->get_bus()->get_cartridge()->rom().empty() &&
          !gb_->get_bus()->get_cartridge()->cgb_enabled();
    }

    [[nodiscard]] static bool is_rom_favorite(const rom& entry) noexcept
    {
        return entry.is_favorite;
    }
};

#endif  // GAMEBOY_FRONTEND_H
