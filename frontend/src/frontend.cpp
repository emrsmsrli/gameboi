#include "frontend.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string_view>
#include <thread>
#include <utility>

#include <spdlog/spdlog.h>

namespace
{

enum main_menu_item {
    resume = 0,
    select_audio_device,
    select_gb_color_palette,
    select_rom_file,
    quit,
    count // don't use
};

using namespace std::string_view_literals;
constexpr std::array gb_palettes{
  std::make_pair("Greyscale"sv, &gameboy::ppu::palette_grayscale),
  std::make_pair("Gold"sv, &gameboy::ppu::palette_gold),
  std::make_pair("Green"sv, &gameboy::ppu::palette_green),
  std::make_pair("Zelda"sv, &gameboy::ppu::palette_zelda)
};

constexpr auto* config_file_name = "config.json";
constexpr auto* config_key_favorite = "favorite";
constexpr auto* config_key_gb_palette_idx = "gb_palette_idx";
constexpr auto* config_key_audio_device = "last_audio_device_id";

} // namespace

using json = nlohmann::json;

frontend::frontend(
  const uint32_t width, const uint32_t height, const bool fullscreen,
  const gameboy::filesystem::path& rom_base_path) noexcept
    : frontend(width, height, fullscreen)
{
    if(gameboy::filesystem::is_directory(rom_base_path)) {
        for(const auto& entry : gameboy::filesystem::directory_iterator{rom_base_path}) {
            if(const auto& path = entry.path();
              entry.is_regular_file() &&
                (path.extension() == ".gb" || path.extension() == ".gbc"))
            {
                const auto file_name = path.filename().string();
                std::optional<int32_t> gb_palette_idx;
                bool is_favorite = false;

                if(auto entry_it = config_.find(file_name); entry_it != config_.end()) {
                    if(auto fav_it = entry_it->find(config_key_favorite); fav_it != entry_it->end()) {
                        is_favorite = fav_it->get<bool>();
                    }

                    if(auto palette_it = entry_it->find(config_key_gb_palette_idx); palette_it != entry_it->end()) {
                        gb_palette_idx = palette_it->get<int32_t>();
                    }
                }

                spdlog::trace("rom: {}", file_name);
                roms_.push_back(rom_entry{path, gb_palette_idx, is_favorite});
            }
        }

        generate_rom_select_menu_items();
    } else if(gameboy::filesystem::is_regular_file(rom_base_path) &&
      (rom_base_path.extension() == ".gb" || rom_base_path.extension() == ".gbc")) {
        state_ = state::game;
        roms_.push_back(rom_entry{rom_base_path, std::nullopt, false});
    } else {
        spdlog::critical("incorrect folder or invalid rom file: {}", rom_base_path.string());
        std::terminate();
    }
}

frontend::frontend(const uint32_t width, const uint32_t height, const bool fullscreen) noexcept
    : config_(
        gameboy::filesystem::exists(config_file_name)
          ? json::parse(gameboy::read_file(config_file_name))
          : json::object()
      ),
      window_(
        sf::VideoMode(width, height),
        "GAMEBOY",
        fullscreen ? sf::Style::Fullscreen : sf::Style::Default
      ),
      audio_device_{
        sdl::audio_device::device_name(config_.contains(config_key_audio_device)
              ? config_[config_key_audio_device].get<int32_t>() : 0), 2u,
        sdl::audio_device::format::s16,
        gameboy::apu::sampling_rate,
        gameboy::apu::sample_size
      },
      menu_title_{"Pick ROM", font_, 45}
{
    menu_bg_.setFillColor(sf::Color{0x111111BB});

    window_.setFramerateLimit(60u);
    window_.setVerticalSyncEnabled(false);
    window_buffer_.create(gameboy::screen_width, gameboy::screen_height, sf::Color::White);
    window_texture_.create(gameboy::screen_width, gameboy::screen_height);

    window_sprite_.setTexture(window_texture_);

    const auto sprite_local_bounds = window_sprite_.getLocalBounds();
    window_sprite_.setOrigin(sprite_local_bounds.width * .5f, sprite_local_bounds.height * .5f);

    rescale_view();
    render_frame();

    audio_device_.resume();

    if(!font_.loadFromFile("res/arcade.ttf")) {
        spdlog::critical("could not load arcade font");
        std::terminate();
    }

    main_menu_.emplace(font_, "Resume").set_selected(true);
    main_menu_.emplace(font_, "Select audio device");
    main_menu_.emplace(font_, "Select GB Color palette");
    main_menu_.emplace(font_, "Select ROM to play");
    main_menu_.emplace(font_, "Quit");

    for(auto& entry : main_menu_) {
        entry.bg().setScale(2.f, 4.f);
    }

    for(const auto& [name, palette] : gb_palettes) {
        select_gb_color_palette_menu_.emplace(font_, std::string{name}, palette);
    }

    main_menu_.on_item_selected({gameboy::connect_arg<&frontend::on_main_menu_item_selected>, this});
    select_audio_device_menu_.on_item_selected({gameboy::connect_arg<&frontend::on_audio_device_selected>, this});
    select_gb_color_palette_menu_.on_item_selected({gameboy::connect_arg<&frontend::on_gb_color_palette_selected>, this});
    select_rom_file_menu_.on_item_selected({gameboy::connect_arg<&frontend::on_rom_file_selected>, this});
}

frontend::~frontend()
{
    std::ofstream config_file{config_file_name};
    config_file << std::setw(4) /*pretty print*/ << config_;
}

void frontend::register_gameboy(const gameboy::observer<gameboy::gameboy> gb) noexcept
{
    gb_ = gb;

    if(state_ == state::game) {
        gb_->load_rom(roms_.front().path);
    }

    gb_->on_render_line({gameboy::connect_arg<&frontend::render_line>, this});
    gb_->on_vblank({gameboy::connect_arg<&frontend::render_frame>, this});
    gb_->on_audio_buffer_full({gameboy::connect_arg<&frontend::play_sound>, this});
}

void frontend::play_sound(const gameboy::apu::sound_buffer& sound_buffer) noexcept
{
    const auto buffer_size_in_bytes = sizeof(gameboy::apu::sound_buffer::value_type) * sound_buffer.size();
    while(audio_device_.queue_size() > buffer_size_in_bytes) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
    }

    audio_device_.enqueue(sound_buffer.data(), buffer_size_in_bytes);
}

void frontend::rescale_view() noexcept
{
    const auto [width, height] = window_.getSize();
    const auto sprite_local_bounds = window_sprite_.getLocalBounds();

    const auto screen_aspect_ratio = static_cast<float>(width) / height;
    const auto sprite_aspect_ratio = sprite_local_bounds.width / sprite_local_bounds.height;
    const auto scale = screen_aspect_ratio > sprite_aspect_ratio
                       ? height / sprite_local_bounds.height
                       : width / sprite_local_bounds.width;

    window_sprite_.setScale(scale, scale);
    window_sprite_.setPosition(width * .5f, height * .5f);

    const auto sprite_global_bounds = window_sprite_.getGlobalBounds();

    menu_view_ = window_.getView();
    menu_view_.setViewport(sf::FloatRect{
      sprite_global_bounds.left / width,
      (sprite_global_bounds.top + sprite_global_bounds.height * .15f) / height,
      sprite_global_bounds.width / width,
      (sprite_global_bounds.height - sprite_global_bounds.height * .15f) / height,
    });

    menu_bg_.setSize(sf::Vector2f(width, height));
    menu_title_.setPosition(sprite_global_bounds.left, sprite_global_bounds.top);

    main_menu_.set_bounds(sf::Vector2f(width, height));
    select_audio_device_menu_.set_bounds(sf::Vector2f(width, height));
    select_gb_color_palette_menu_.set_bounds(sf::Vector2f(width, height));
    select_rom_file_menu_.set_bounds(sf::Vector2f(width, height));

    draw_sprite();
}

void frontend::render_line(const uint8_t line_number, const gameboy::render_line& line) noexcept
{
    for(size_t i = 0; i < line.size(); ++i) {
        const auto& color = line[i];
        window_buffer_.setPixel(i, line_number, {
          color.red, color.green, color.blue, 255
        });
    }
}

void frontend::draw_sprite() noexcept
{
    window_.clear();
    window_.draw(window_sprite_);
    window_.display();
}

void frontend::render_frame() noexcept
{
    window_texture_.update(window_buffer_);
    draw_sprite();
}

frontend::tick_result frontend::tick()
{
    if(state_ == state::quitting) {
        return tick_result::should_quit;
    }

    while(window_.pollEvent(event_)) {
        if(event_.type == sf::Event::Closed ||
          (event_.type == sf::Event::KeyReleased && event_.key.code == sf::Keyboard::Q)) {
            window_.close();
            return tick_result::should_quit;
        }

        if(event_.type == sf::Event::Resized) {
            const sf::FloatRect visible_area(0, 0, event_.size.width, event_.size.height);
            window_.setView(sf::View{visible_area});
            rescale_view();
        } else {
            switch(state_) {
                case state::game: {
                    if(event_.type == sf::Event::KeyReleased && event_.key.code == sf::Keyboard::Escape) {
                        state_ = state::main_menu;
                        menu_title_.setString("Main Menu");
                        main_menu_[2].set_disabled(!can_pick_gb_color_palette());
                    } else {
                        handle_game_keys(event_);
                    }
                    break;
                }
                case state::main_menu: {
                    main_menu_.on_key_event(event_);
                    break;
                }
                case state::select_audio_device: {
                    select_audio_device_menu_.on_key_event(event_);
                    break;
                }
                case state::select_gb_color_palette: {
                    select_gb_color_palette_menu_.on_key_event(event_);
                    break;
                }
                case state::select_rom_file: {
                    if(!select_rom_file_menu_.on_key_event(event_) &&
                      event_.type == sf::Event::KeyReleased &&
                      (event_.key.code == sf::Keyboard::Left || event_.key.code == sf::Keyboard::Right)) {
                        auto& rom = roms_[select_rom_file_menu_.get_current_idx()];
                        auto& is_favorite = rom.is_favorite;

                        is_favorite = !is_favorite;
                        config_[rom.path.filename().string()][config_key_favorite] = is_favorite;

                        if(is_favorite) {
                            select_rom_file_menu_[select_rom_file_menu_.get_current_idx()]
                              .text().setFillColor(sf::Color{0xFFFF00FF});
                        } else {
                            select_rom_file_menu_[select_rom_file_menu_.get_current_idx()]
                              .text().setFillColor(sf::Color::White);
                        }
                    }
                    break;
                }
                default:
                case state::quitting:
                    return tick_result::should_quit;
            }
        }
    }

    const auto draw_menu = [&](auto& menu) {
      window_.clear(sf::Color::Black);
      window_.draw(window_sprite_);
      window_.draw(menu_bg_);

      window_.draw(menu_title_);

      const auto prev_view = window_.getView();
      window_.setView(menu_view_);
      menu.draw(window_);
      window_.setView(prev_view);

      window_.display();
    };

    switch(state_) {
        case state::main_menu:
            draw_menu(main_menu_);
            break;
        case state::select_audio_device:
            draw_menu(select_audio_device_menu_);
            break;
        case state::select_gb_color_palette:
            draw_menu(select_gb_color_palette_menu_);
            break;
        case state::select_rom_file:
            draw_menu(select_rom_file_menu_);
            break;

        case state::game:
            return tick_result::ticking;

        default:
            break;
    }

    return tick_result::paused;
}

void frontend::on_main_menu_item_selected(const size_t idx) noexcept
{
    switch(idx) {
        case main_menu_item::resume:
            state_ = state::game;
            break;
        case main_menu_item::select_audio_device:
            select_audio_device_menu_.clear();
            for(auto i = 0; i < sdl::audio_device::num_devices(); ++i) {
                select_audio_device_menu_.emplace(font_, std::string{sdl::audio_device::device_name(i)}, 20.f);
            }
            select_audio_device_menu_[0].set_selected(true);

            menu_title_.setString("Select Device");
            state_ = state::select_audio_device;
            break;
        case main_menu_item::select_gb_color_palette:
            menu_title_.setString("Select Palette");
            state_ = state::select_gb_color_palette;
            break;
        case main_menu_item::select_rom_file:
            generate_rom_select_menu_items();
            menu_title_.setString("Select ROM");
            state_ = state::select_rom_file;
            break;
        case main_menu_item::quit:
        default:
            window_.close();
            state_ = state::quitting;
            break;
    }

    // todo set relevant items disabled or enabled
}

void frontend::on_audio_device_selected(const size_t idx) noexcept
{
    state_ = state::game;
    config_[config_key_audio_device] = idx;

    audio_device_ = sdl::audio_device{
      sdl::audio_device::device_name(idx),
      2u, sdl::audio_device::format::s16,
      gameboy::apu::sampling_rate,
      gameboy::apu::sample_size
    };
    audio_device_.resume();
}

void frontend::on_gb_color_palette_selected(const size_t idx) noexcept
{
    state_ = state::game;
    gb_->set_gb_palette(*gb_palettes[idx].second);
    config_
      [gb_->get_bus()->get_cartridge()->get_rom_path().filename().string()]
      [config_key_gb_palette_idx] = idx;
}

void frontend::on_rom_file_selected(const size_t idx) noexcept
{
    state_ = state::game;
    const auto cartridge = gb_->get_bus()->get_cartridge();
    const auto& rom_path = roms_[idx].path;
    if(cartridge->get_rom_path() == rom_path) {
        return;
    }

    gb_->load_rom(rom_path);
    window_.setTitle(fmt::format("GAMEBOY - {}", gb_->rom_name()));

    if(!cartridge->cgb_enabled()) {
        auto& rom_entry = config_[rom_path.filename().string()];
        if(auto palette_it = rom_entry.find(config_key_gb_palette_idx); palette_it != rom_entry.end()) {
            gb_->set_gb_palette(*gb_palettes[palette_it->get<int32_t>()].second);
        }
    }

    if(on_new_rom_) {
        on_new_rom_();
    }
}

void frontend::generate_rom_select_menu_items() noexcept
{
    std::stable_partition(begin(roms_), end(roms_), is_rom_favorite);

    select_rom_file_menu_.clear();
    std::for_each(begin(roms_), end(roms_), [&](const rom_entry& e) {
      auto& menu_entry = select_rom_file_menu_.emplace(font_, e.path.filename().string(), 18);
      if(e.is_favorite) {
          menu_entry.text().setFillColor(sf::Color{0xFFFF00FF});
      }
    });
    select_rom_file_menu_[0].set_selected(true);
}

void frontend::handle_game_keys(const sf::Event& key_event) noexcept
{
    if(key_event.type == sf::Event::KeyPressed) {
        switch(key_event.key.code) {
            case sf::Keyboard::Up:
                gb_->press_key(gameboy::joypad::key::up);
                break;
            case sf::Keyboard::Down:
                gb_->press_key(gameboy::joypad::key::down);
                break;
            case sf::Keyboard::Left:
                gb_->press_key(gameboy::joypad::key::left);
                break;
            case sf::Keyboard::Right:
                gb_->press_key(gameboy::joypad::key::right);
                break;
            case sf::Keyboard::Z:
                gb_->press_key(gameboy::joypad::key::a);
                break;
            case sf::Keyboard::X:
                gb_->press_key(gameboy::joypad::key::b);
                break;
            case sf::Keyboard::Enter:
                gb_->press_key(gameboy::joypad::key::start);
                break;
            case sf::Keyboard::Space:
                gb_->press_key(gameboy::joypad::key::select);
                break;
#if WITH_DEBUGGER
            case sf::Keyboard::F:
            case sf::Keyboard::F7:
                gb_->tick();
                break;
#endif //WITH_DEBUGGER
            default:
                break;
        }
    } else if(key_event.type == sf::Event::KeyReleased) {
        switch(key_event.key.code) {
            case sf::Keyboard::Up:
                gb_->release_key(gameboy::joypad::key::up);
                break;
            case sf::Keyboard::Down:
                gb_->release_key(gameboy::joypad::key::down);
                break;
            case sf::Keyboard::Left:
                gb_->release_key(gameboy::joypad::key::left);
                break;
            case sf::Keyboard::Right:
                gb_->release_key(gameboy::joypad::key::right);
                break;
            case sf::Keyboard::Z:
                gb_->release_key(gameboy::joypad::key::a);
                break;
            case sf::Keyboard::X:
                gb_->release_key(gameboy::joypad::key::b);
                break;
            case sf::Keyboard::Enter:
                gb_->release_key(gameboy::joypad::key::start);
                break;
            case sf::Keyboard::Space:
                gb_->release_key(gameboy::joypad::key::select);
                break;
            case sf::Keyboard::S:
                gb_->save_ram_rtc();
                break;
#if WITH_DEBUGGER
            case sf::Keyboard::G:
                gb_->tick_one_frame();
                break;
            case sf::Keyboard::T:
            case sf::Keyboard::F9:
                gb_->tick_enabled = !gb_->tick_enabled;
                break;
#endif //WITH_DEBUGGER
            default:
                break;
        }
    }
}
