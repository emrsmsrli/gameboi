#ifndef GAMEBOY_OBSERVER_H
#define GAMEBOY_OBSERVER_H

#include <type_traits>
#include <memory>

namespace gameboy {
    template<typename ElementType>
    class Observer {
    public:
        constexpr Observer() noexcept = default;
        constexpr Observer(std::nullptr_t) noexcept = default;
        explicit Observer(ElementType* value)
                :ptr(value) { }

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
    Observer<W> make_observer(W* p) noexcept
    {
        return Observer<W>{p};
    }

    template<typename T>
    bool operator==(const Observer<T>& l, const Observer<T>& r)
    {
        return l.get() == r.get();
    }
}

#endif //GAMEBOY_OBSERVER_H
