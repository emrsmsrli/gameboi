#ifndef GAMEBOY_OBSERVER_H
#define GAMEBOY_OBSERVER_H

#include <type_traits>
#include <memory>

namespace gameboy {

template<typename ElementType>
class observer {
public:
    constexpr observer() noexcept = default;
    constexpr observer(std::nullptr_t) noexcept
        : observer(nullptr) {};
    explicit observer(ElementType* ptr)
        : ptr_(ptr) {}

    constexpr void reset(ElementType* ptr = nullptr) noexcept { ptr_ = ptr; }
    constexpr ElementType* release() noexcept
    {
        auto* ret = ptr_;
        ptr_ = nullptr;
        return ret;
    }

    constexpr ElementType* get() const noexcept { return ptr_; }

    constexpr explicit operator bool() const noexcept { return ptr_ != nullptr; }
    constexpr explicit operator ElementType() const noexcept { return ptr_; }

    constexpr ElementType* operator->() const noexcept { return ptr_; }
    constexpr std::add_lvalue_reference_t<ElementType> operator*() const { return *ptr_; }

private:
    ElementType* ptr_ = nullptr;
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
