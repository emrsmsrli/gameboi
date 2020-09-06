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

struct button {
    sf::Text text;
    sf::RectangleShape bg;
    bool disabled = false;

    button(const sf::Font& f, const std::string& t, const uint32_t character_size = 30)
      : text{t, f, character_size},
        bg{{text.getGlobalBounds().width, text.getGlobalBounds().height}}
    {
        text.setFillColor(sf::Color::White);
        bg.setFillColor(sf::Color::Black);
        bg.setOutlineColor(sf::Color{0x575757FF});
        bg.setOutlineThickness(.2f);
        bg.setScale(2.f, 2.f);
    }

    void draw(sf::RenderWindow& w, const float y_offset) noexcept
    {
        detail::do_draw(w, bg, sf::Vector2f{0.f, y_offset});
        detail::do_draw(w, text, sf::Vector2f{4.f, y_offset});
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
    template<typename ...TArgs>
    TEntry& emplace(TArgs&&... args) noexcept
    {
        auto& entry = entries_.emplace_back(std::forward<TArgs>(args)...);
        entry.set_width(bounds_.x);
        return entry;
    }

    void clear() noexcept { entries_.clear(); }

    void draw(sf::RenderWindow& window) noexcept
    {
        auto start_y = scroll_offset_;
        for(auto& entry : entries_) {
            entry.draw(window, start_y);
            start_y += entry.height() + 2.f;
        }
    }

    void on_item_selected(gameboy::delegate<void(size_t)> on_item_selected) noexcept { on_item_selected_ = on_item_selected; }

    bool on_key_event(const sf::Event& event) noexcept
    {
        const auto update_current_selected = [&](const size_t new_current) {
          std::size_t idx = 0;
          for(auto& entry : entries_) {
              entry.set_selected(new_current == idx);
              ++idx;
          }

          const float new_start_y = -(entries_[0].height() + 2.f) * new_current;
          if(new_start_y > scroll_offset_) {
              scroll_offset_ += entries_[0].height() + 2.f;
          } else if(new_start_y - entries_[0].height() < scroll_offset_ - bounds_.y) {
              scroll_offset_ -= entries_[0].height() + 2.f;
          }

          current_idx_ = new_current;
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
                        update_current_selected(std::distance(it, rend) - 1);
                    }
                    break;
                }
                case sf::Keyboard::Down: {
                    auto it = std::find_if(entries_.begin() + current_idx_ + 1, entries_.end(), [](const TEntry& e) {
                      return !e.disabled;
                    });

                    if(it != entries_.end()) {
                        update_current_selected(std::distance(entries_.begin(), it));
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

                    update_current_selected(0);
                    scroll_offset_ = 0.f;
                    break;
                }
                default:
                    return false;
            }
        }

        return true;
    }

    void set_bounds(const sf::Vector2f& bounds) noexcept
    {
        bounds_ = bounds;
        for(auto& entry : entries_) {
            entry.set_width(bounds.x);
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
    sf::Vector2f bounds_;
    std::vector<TEntry> entries_;
    int32_t current_idx_ = 0;
    float scroll_offset_ = 0.f;
};

} // namespace ui

#endif  // GAMEBOY_LIST_VIEW_H
