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
    explicit observer(ElementType* value)
        : ptr(value) {}

    constexpr void reset(ElementType* value = nullptr) noexcept { ptr = value; }
    constexpr ElementType* release() noexcept
    {
        auto* ret = ptr;
        ptr = nullptr;
        return ret;
    }

    constexpr ElementType* get() const noexcept { return ptr; }

    constexpr explicit operator bool() const noexcept { return ptr != nullptr; }
    constexpr explicit operator ElementType() const noexcept { return ptr; }

    constexpr ElementType* operator->() const noexcept { return ptr; }
    constexpr std::add_lvalue_reference_t<ElementType> operator*() const { return *ptr; }

private:
    ElementType* ptr = nullptr;
};

template<class W>
observer<W> make_observer(W* p) noexcept
{
    return observer<W>{p};
}

template<typename T>
bool operator==(const observer<T>& l, const observer<T>& r)
{
    return l.get() == r.get();
}

}

#endif //GAMEBOY_OBSERVER_H
