#ifndef GAMEBOY_LIST_VIEW_H
#define GAMEBOY_LIST_VIEW_H

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <string>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace ui {

namespace detail {

template<typename T>
void do_draw(sf::RenderWindow& w, T& t, const sf::Vector2f& offset) {
    const auto pos = t.getPosition();
    t.setPosition(pos + offset);
    w.draw(t);
    t.setPosition(pos);
}

} // namespace detail

struct list_button {
    sf::Text text;
    sf::RectangleShape bg;
    bool disabled = false;

    list_button(const sf::Font& f, const std::string& t, const uint32_t character_size = 30)
      : text{t, f, character_size},
        bg{{text.getGlobalBounds().width, text.getGlobalBounds().height}}
    {
        text.setFillColor(sf::Color::White);
        bg.setFillColor(sf::Color::Black);
        bg.setOutlineColor(sf::Color{0x575757FF});
        bg.setOutlineThickness(.2f);
        bg.setScale(2.f, 2.f);
    }

    void draw(sf::RenderWindow& w, const sf::Vector2f& offset) noexcept
    {
        detail::do_draw(w, bg, offset);
        detail::do_draw(w, text, offset + sf::Vector2f{3.f, 0.f});
    }

    void set_width(const float width) noexcept { bg.setSize({width, bg.getSize().y}); }

    void set_disabled(const bool d) noexcept
    {
        disabled = d;
        if(disabled) {
            text.setFillColor(sf::Color{128, 128, 128, 255});
        } else {
            text.setFillColor(sf::Color::White);
        }
    }

    void set_selected(const bool selected) noexcept
    {
        if(selected) {
            bg.setFillColor(sf::Color{0x343434FF});
        } else {
            bg.setFillColor(sf::Color::Black);
        }
    }

    float height() const noexcept { return bg.getGlobalBounds().height; }
};

template<typename TEntry>
class list_view {
public:
    list_view() = default;
    list_view(std::initializer_list<TEntry> entries)
      : entries_{entries}
    {
        if(!entries_.empty()) {
            entries_.front().set_selected(true);
        }
    }

    template<typename ...TArgs>
    TEntry& emplace(TArgs&&... args) noexcept
    {
        auto& entry = entries_.emplace_back(std::forward<TArgs>(args)...);
        entry.set_width(width_);
        return entry;
    }

    void clear() noexcept { entries_.clear(); }

    void draw(sf::RenderWindow& window) noexcept
    {
        auto start_pos = position_;
        for(auto& entry : entries_) {
            entry.draw(window, start_pos - sf::Vector2f{0.f, 600.f});
            start_pos.y += entry.height() + 2.f;
        }

        // todo draw scrollbar
    }

    void on_item_selected(gameboy::delegate<void(size_t)> on_item_selected) noexcept { on_item_selected_ = on_item_selected; }

    bool on_key_event(const sf::Event& event) noexcept
    {
        const auto update_current_selected = [&]() {
          std::size_t idx = 0;
          for(auto& entry : entries_) {
              entry.set_selected(current_idx_ == idx);
              ++idx;
          }
        };

        if(event.type == sf::Event::KeyPressed) {
            switch(event.key.code) {
                case sf::Keyboard::Up: {
                    const auto make_reverse_it = [](auto fwd_it) {
                      return std::reverse_iterator<decltype(fwd_it)>(fwd_it);
                    };

                    const auto rbegin = make_reverse_it(entries_.end()) + (entries_.size() - current_idx_);
                    const auto rend = make_reverse_it(entries_.begin());
                    auto it = std::find_if(rbegin, rend, [](const TEntry& e) {
                      return !e.disabled;
                    });

                    if(it != rend) {
                        current_idx_ = std::distance(it, rend) - 1;
                        update_current_selected();
                    }
                    break;
                }
                case sf::Keyboard::Down: {
                    auto it = std::find_if(entries_.begin() + current_idx_ + 1, entries_.end(), [](const TEntry& e) {
                      return !e.disabled;
                    });

                    if(it != entries_.end()) {
                        current_idx_ = std::distance(entries_.begin(), it);
                        update_current_selected();
                    }
                    break;
                }
                default:
                    return false;
            }
        } else if(event.type == sf::Event::KeyReleased) {
            switch(event.key.code) {
                case sf::Keyboard::Enter: {
                    if(on_item_selected_) {
                        on_item_selected_(current_idx_);
                    }

                    current_idx_ = 0;
                    update_current_selected();
                    break;
                }
                default:
                    return false;
            }
        }

        return true;
    }

    void set_width(const float width) noexcept
    {
        width_ = width;
        for(auto& entry : entries_) {
            entry.set_width(width);
        }
    }

    [[nodiscard]] int32_t get_current_idx() const noexcept { return current_idx_; }

    [[nodiscard]] const TEntry& operator[](const size_t idx) const noexcept { return entries_[idx]; }
    [[nodiscard]] TEntry& operator[](const size_t idx) noexcept { return entries_[idx]; }

    auto begin() noexcept { return entries_.begin(); }
    auto cbegin() const noexcept { return entries_.cbegin(); }
    auto end() noexcept { return entries_.end(); }
    auto cend() const noexcept { return entries_.cend(); }

private:
    gameboy::delegate<void(size_t)> on_item_selected_;
    sf::Vector2f position_;
    std::vector<TEntry> entries_;
    int32_t current_idx_ = 0;
    float width_ = 0.f;
};

} // namespace ui

#endif  // GAMEBOY_LIST_VIEW_H
