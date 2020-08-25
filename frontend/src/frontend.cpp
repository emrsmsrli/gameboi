#include "frontend.h"

#include <chrono>
#include <thread>

#include "imgui-SFML.h"
#include "imgui.h"
#include "sdl_core.h"

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

template<typename T>
void draw_fullscreen_imgui_window(const char* tag, const char* title, const sf::Window& window, T&& draw_func)
{
    ImGui::SetNextWindowPos(ImVec2{0.f, 0.f});
    ImGui::SetNextWindowSize(ImVec2(window.getSize().x, window.getSize().y));

    if(ImGui::Begin(tag, nullptr,
      ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize)) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

        for(int i = 0; i < 2; ++i) {
            ImGui::Spacing();
        }

        ImGui::TextUnformatted(title);

        for(int i = 0; i < 6; ++i) {
            ImGui::Spacing();
        }

        draw_func();

        ImGui::PopFont();
        ImGui::End();
    }
}

} // namespace

frontend::frontend(
  const uint32_t width, const uint32_t height, const bool fullscreen,
  const gameboy::filesystem::path& rom_base_path) noexcept
    : frontend(width, height, fullscreen)
{
    // todo check ini file for favorites

    if(gameboy::filesystem::is_directory(rom_base_path)) {
        for(const auto& entry : gameboy::filesystem::directory_iterator{rom_base_path}) {
            if(const auto& path = entry.path();
              entry.is_regular_file() &&
                (path.extension() == ".gb" || path.extension() == ".gbc"))
            {
                spdlog::trace("rom file found {}", path.string());
                rom_files_.push_back(path);

                auto save = path;
                save.replace_extension(".sav");
                if(gameboy::filesystem::exists(save)) {
                    saved_rom_files_.push_back(path);
                    spdlog::trace("\tsave file found");
                }
            }
        }

        menu_selected_index_ = 0;
        menu_max_index_ = rom_files_.size();
    } else if(gameboy::filesystem::is_regular_file(rom_base_path) &&
      (rom_base_path.extension() == ".gb" || rom_base_path.extension() == ".gbc")) {
        state_ = state::game;
        saved_rom_files_.push_back(rom_base_path);
    } else {
        spdlog::critical("incorrect folder or invalid rom file: {}", rom_base_path.string());
        std::terminate();
    }
}

frontend::frontend(const uint32_t width, const uint32_t height, const bool fullscreen) noexcept
    : window_(
      sf::VideoMode(width, height),
      "GAMEBOY",
      fullscreen ? sf::Style::Fullscreen : sf::Style::Default
    ),
      audio_device_{
      sdl::audio_device::device_name(0), 2u,
      sdl::audio_device::format::s16,
      gameboy::apu::sampling_rate,
      gameboy::apu::sample_size
    }
{
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

    ImGui::SFML::Init(window_);

    ImGui::GetIO().Fonts->AddFontFromFileTTF("res/arcade.ttf", 18.f);
    ImGui::SFML::UpdateFontTexture();
}

frontend::~frontend()
{
    ImGui::SFML::Shutdown();
}

void frontend::register_gameboy(const gameboy::observer<gameboy::gameboy> gb) noexcept
{
    gb_ = gb;

    if(state_ == state::game) {
        gb_->load_rom(rom_files_.front());
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
    while(window_.pollEvent(event_)) {
        ImGui::SFML::ProcessEvent(event_);

        if(event_.type == sf::Event::Closed ||
          (event_.type == sf::Event::KeyReleased && event_.key.code == sf::Keyboard::Q)) {
            window_.close();
            return tick_result::should_exit;
        }

        if(event_.type == sf::Event::Resized) {
            const sf::FloatRect visible_area(0, 0, event_.size.width, event_.size.height);
            window_.setView(sf::View{visible_area});
            rescale_view();
        }

        if(state_ == state::game) {
            handle_game_keys(event_);

            if(event_.type == sf::Event::KeyReleased) {
                if(event_.key.code == sf::Keyboard::Escape) {
                    state_ = state::main_menu;
                    menu_selected_index_ = main_menu_item::resume;
                    menu_max_index_ = main_menu_item::count;
                }
            }
        } else {
            if(event_.type == sf::Event::KeyPressed) {
                if(event_.key.code == sf::Keyboard::Down && menu_selected_index_ < menu_max_index_ - 1) {
                    ++menu_selected_index_;

                    if(state_ == state::main_menu &&
                      menu_selected_index_ == main_menu_item::select_gb_color_palette &&
                      !can_pick_gb_color_palette()) {
                        ++menu_selected_index_;
                    }
                } else if(event_.key.code == sf::Keyboard::Up && menu_selected_index_ > 0) {
                    --menu_selected_index_;

                    if(state_ == state::main_menu &&
                      menu_selected_index_ == main_menu_item::select_gb_color_palette &&
                      !can_pick_gb_color_palette()) {
                        --menu_selected_index_;
                    }
                }
            } else if(event_.type == sf::Event::KeyReleased && event_.key.code == sf::Keyboard::Enter) {
                const auto prev_state = state_;
                const auto prev_selected_item = menu_selected_index_;

                state_ = state::main_menu;
                menu_selected_index_ = main_menu_item::resume;
                menu_max_index_ = main_menu_item::count;

                switch(prev_state) {
                    case state::main_menu: {
                        switch(prev_selected_item) {
                            case main_menu_item::resume:
                                state_ = state::game;
                                break;
                            case main_menu_item::select_audio_device:
                                menu_max_index_ = sdl::audio_device::num_devices();
                                state_ = state::select_audio_device;
                                break;
                            case main_menu_item::select_gb_color_palette:
                                menu_max_index_ = gb_palettes.size();
                                state_ = state::select_gb_color_palette;
                                break;
                            case main_menu_item::select_rom_file:
                                menu_max_index_ = rom_files_.size();
                                state_ = state::select_rom_file;
                                break;
                            case main_menu_item::quit:
                            default:
                                window_.close();
                                return tick_result::should_exit;
                        }
                        break;
                    }
                    case state::select_audio_device: {
                        audio_device_ = sdl::audio_device{
                            sdl::audio_device::device_name(menu_selected_index_),
                            2u, sdl::audio_device::format::s16,
                            gameboy::apu::sampling_rate,
                            gameboy::apu::sample_size
                        };
                        break;
                    }
                    case state::select_gb_color_palette: {
                        gb_->set_gb_palette(*gb_palettes[menu_selected_index_].second);
                        break;
                    }
                    case state::select_rom_file: {
                        gb_->load_rom(rom_files_[menu_selected_index_]);  // fixme
                        window_.setTitle(fmt::format("GAMEBOY - {}", gb_->rom_name()));

                        if(on_new_rom_) {
                            on_new_rom_();
                        }
                        break;
                    }
                    case state::game:
                    default:
                        break;
                }
            }
        }
    }

    const auto delta = dt_.restart();
    ImGui::SFML::Update(window_, delta);

    switch(state_) {
        case state::main_menu:
            draw_menu();
            break;
        case state::select_audio_device:
            draw_audio_device_select();
            break;
        case state::select_gb_color_palette:
            draw_gb_palette_select();
            break;
        case state::select_rom_file:
            draw_rom_select();
            break;

        case state::game:
        default:
            break;
    }

    window_.clear(sf::Color::Black);
    window_.draw(window_sprite_);
    ImGui::SFML::Render(window_);
    window_.display();

    return state_ == state::game
      ? tick_result::ticking
      : tick_result::paused;
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

void frontend::draw_menu() noexcept
{
    static constexpr std::array menu_items{
        std::make_pair(main_menu_item::resume, "Resume"),
        std::make_pair(main_menu_item::select_audio_device, "Select audio device"),
        std::make_pair(main_menu_item::select_gb_color_palette, "Select GB Color palette"),
        std::make_pair(main_menu_item::select_rom_file, "Select ROM to play"),
        std::make_pair(main_menu_item::quit, "Quit")
    };

    draw_fullscreen_imgui_window("##menu", "", window_, [&]() {
        for(const auto& [key, name] : menu_items) {
            ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
            if(key == main_menu_item::select_gb_color_palette && !can_pick_gb_color_palette()) {
                flags = ImGuiSelectableFlags_Disabled;
            }

            const auto selected = menu_selected_index_ == key;
            ImGui::Selectable(name, selected, flags, ImVec2{0.f, 80.f});
            if(selected) {
                ImGui::SetScrollHereY();
            }
        }
    });
}

void frontend::draw_audio_device_select() noexcept
{
    draw_fullscreen_imgui_window("##select_audio_device", "Select an audio device:", window_, [&]() {
        for(auto i = 0; i < sdl::audio_device::num_devices(); ++i) {
            const auto selected = menu_selected_index_ == i;
            ImGui::Selectable(sdl::audio_device::device_name(i).data(), selected);
            if(selected) {
                ImGui::SetScrollHereY();
            }
        }
    });
}

void frontend::draw_gb_palette_select() noexcept
{
    draw_fullscreen_imgui_window("##select_gb_palette", "Select a color palette:", window_, [&]() {
        auto idx = 0;
        for(const auto& [name, palette] : gb_palettes) {
            ImGui::Selectable(name.data(), idx == menu_selected_index_, ImGuiSelectableFlags_None, ImVec2{0.f, 80.f});

            const auto cursor = ImGui::GetCursorPos();
            ImGui::SetCursorPos(ImVec2{cursor.x, cursor.y - 60.f});

            for(const auto& color : palette->colors) {
                const ImVec4 color_float{color.red / 255.f, color.green / 255.f, color.blue / 255.f, 1.f};

                ImGui::ColorButton("", color_float, ImGuiColorEditFlags_NoPicker, ImVec2{50.f, 50.f});
                ImGui::SameLine(0, 10.f);
            }

            ImGui::SetCursorPos(ImVec2{cursor.x, cursor.y - 50.f});

            ImGui::NewLine();
            ++idx;
        }
    });
}

void frontend::draw_rom_select() noexcept
{
    // todo make 2 tabs: with save files and all roms
    draw_fullscreen_imgui_window("##select_rom", "Select a rom:", window_, [&]() {
        ImGuiListClipper clipper(rom_files_.size());
        while(clipper.Step()) {
            for(auto i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) {
                const auto selected = menu_selected_index_ == i;
                auto name = rom_files_[i].filename();
                name.replace_extension();

                ImGui::Selectable(name.string().c_str(), selected);
                if(selected) {
                    ImGui::SetScrollHereY();
                }
            }
        }
    });
}
