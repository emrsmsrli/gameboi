#ifndef GAMEBOY_OBSERVER_H
#define GAMEBOY_OBSERVER_H

#include <type_traits>
#include <memory>

namespace gameboy {

template<typename ElementType>
class observer {
public:
    using type = ElementType;
    using pointer_type = std::add_pointer_t<ElementType>;
    using const_pointer_type = std::add_const_t<pointer_type>;
    using lvalue_type = std::add_lvalue_reference_t<ElementType>;
    using const_lvalue_type = std::add_const_t<lvalue_type>;

    observer() noexcept = default;
    explicit observer(pointer_type ptr) noexcept
        : ptr_{ptr} {}

    [[nodiscard]] pointer_type get() noexcept { return ptr_; }
    [[nodiscard]] const_pointer_type get() const noexcept { return ptr_; }

    explicit operator bool() const noexcept { return ptr_ != nullptr; }
    explicit operator pointer_type() noexcept { return ptr_; }
    explicit operator const_pointer_type() const noexcept { return ptr_; }

    pointer_type operator->() noexcept { return ptr_; }
    const_pointer_type operator->() const noexcept { return ptr_; }
    [[nodiscard]] lvalue_type operator*() noexcept { return *ptr_; }
    [[nodiscard]] const_lvalue_type operator*() const noexcept { return *ptr_; }

private:
    pointer_type ptr_ = nullptr;
};

template<class W>
observer<W> make_observer(W* ptr) noexcept
{
    return observer<W>{ptr};
}

template<class W>
observer<W> make_observer(W& value) noexcept
{
    return observer<W>{&value};
}

template<typename T>
bool operator==(const observer<T>& l, const observer<T>& r) noexcept
{
    return l.get() == r.get();
}

} // namespace gameboy

#endif //GAMEBOY_OBSERVER_H
