#ifndef GAMEBOY_FRONTEND_H
#define GAMEBOY_FRONTEND_H

#include <cstdint>
#include <optional>
#include <vector>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Window/Event.hpp>
#include <nlohmann/json.hpp>

#include "gameboy/gameboy.h"
#include "list_view.h"
#include "sdl_audio.h"

class frontend {
public:
    enum class tick_result { ticking, paused, should_quit };

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

    [[nodiscard]] tick_result tick();

    void play_sound(const gameboy::apu::sound_buffer& sound_buffer) noexcept;
    void rescale_view() noexcept;
    void render_line(uint8_t line_number, const gameboy::render_line& line) noexcept;
    void draw_sprite() noexcept;
    void render_frame() noexcept;

    void register_gameboy(gameboy::observer<gameboy::gameboy> gb) noexcept;

private:
    struct rom_entry {
        gameboy::filesystem::path path;
        std::optional<uint32_t> gb_palette_idx;
        bool is_favorite;
    };

    enum class state {
        game,
        main_menu,
        select_audio_device,
        select_gb_color_palette,
        select_rom_file,
        quitting
    };

    nlohmann::json config_;
    std::vector<rom_entry> roms_;

    gameboy::observer<gameboy::gameboy> gb_;
    sf::Image window_buffer_;
    sf::Texture window_texture_;
    sf::Sprite window_sprite_;
    sf::RenderWindow window_;
    sf::Font font_;
    sf::Event event_{};

    sdl::audio_device audio_device_;

    sf::Text menu_title_;
    sf::RectangleShape menu_bg_;
    sf::View menu_view_;

    ui::list_view<ui::list_button> main_menu_;
    ui::list_view<ui::list_button> select_audio_device_menu_;
    ui::list_view<ui::list_button> select_gb_color_palette_menu_;
    ui::list_view<ui::list_button> select_rom_file_menu_;

    state state_{state::select_rom_file};

    on_new_rom_func on_new_rom_;

    void on_main_menu_item_selected(size_t idx) noexcept;
    void on_audio_device_selected(size_t idx) noexcept;
    void on_gb_color_palette_selected(size_t idx) noexcept;
    void on_rom_file_selected(size_t idx) noexcept;

    void generate_rom_select_menu_items() noexcept;

    void handle_game_keys(const sf::Event& key_event) noexcept;

    [[nodiscard]] bool can_pick_gb_color_palette() noexcept
    {
        return gb_ &&
          !gb_->get_bus()->get_cartridge()->rom().empty() &&
          !gb_->get_bus()->get_cartridge()->cgb_enabled();
    }

    [[nodiscard]] static bool is_rom_favorite(const rom_entry& entry) noexcept
    {
        return entry.is_favorite;
    }
};

#endif  // GAMEBOY_FRONTEND_H
