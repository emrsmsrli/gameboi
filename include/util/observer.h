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
    using lvalue_type = std::add_lvalue_reference_t<ElementType>;

    constexpr observer() noexcept = default;
    constexpr observer(std::nullptr_t) noexcept
        : observer(nullptr) {};
    explicit observer(pointer_type ptr)
        : ptr_(ptr) {}

    constexpr void reset(pointer_type ptr = nullptr) noexcept { ptr_ = ptr; }
    constexpr pointer_type release() noexcept
    {
        auto* ret = ptr_;
        ptr_ = nullptr;
        return ret;
    }

    constexpr pointer_type get() const noexcept { return ptr_; }

    constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }
    constexpr explicit operator pointer_type() const noexcept { return ptr_; }

    constexpr pointer_type operator->() const noexcept { return ptr_; }
    constexpr lvalue_type operator*() const { return *ptr_; }

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
bool operator==(const observer<T>& l, const observer<T>& r)
{
    return l.get() == r.get();
}

}

#endif //GAMEBOY_OBSERVER_H
