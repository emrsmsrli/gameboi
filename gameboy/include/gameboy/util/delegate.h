#ifndef GAMEBOY_DELEGATE_H
#define GAMEBOY_DELEGATE_H

#include <tuple>
#include <utility>
#include <algorithm>
#include <functional>
#include <type_traits>

namespace gameboy {

namespace internal {

template<typename Ret, typename... Args>
auto to_function_pointer(Ret(*)(Args...)) -> Ret(*)(Args...);

template<typename Ret, typename... Args, typename Type, typename Payload,
    typename = std::enable_if_t<std::is_convertible_v<const Payload*, const Type*>>>
auto to_function_pointer(Ret(*)(Type&, Args...), const Payload*) -> Ret(*)(Args...);

template<typename Ret, typename... Args, typename Type, typename Payload,
    typename = std::enable_if_t<std::is_convertible_v<const Payload*, const Type*>>>
auto to_function_pointer(Ret(*)(Type*, Args...), const Payload*) -> Ret(*)(Args...);

template<typename Class, typename Ret, typename... Args>
auto to_function_pointer(Ret(Class::*)(Args...), const Class*) -> Ret(*)(Args...);

template<typename Class, typename Ret, typename... Args>
auto to_function_pointer(Ret(Class::*)(Args...) const, const Class*) -> Ret(*)(Args...);

template<typename Class, typename Type>
auto to_function_pointer(Type Class::*, const Class*) -> Type(*)();

template<typename... Type>
using to_function_pointer_t = decltype(internal::to_function_pointer(std::declval<Type>()...));


template<typename>
struct function_extent;

template<typename Ret, typename... Args>
struct function_extent<Ret(*)(Args...)> {
    static constexpr auto value = sizeof...(Args);
};

template<typename Func>
constexpr auto function_extent_v = function_extent<Func>::value;

} // namespace internal

template<auto>
struct connect_arg_t {};

template<auto Func>
constexpr connect_arg_t<Func> connect_arg{};

template<typename>
class delegate;

template<typename Ret, typename... Args>
class delegate<Ret(Args...)> {
    using proto_fn_type = Ret(const void*, std::tuple<Args&& ...>);

    template<auto Function, std::size_t... Index>
    void connect(std::index_sequence<Index...>) noexcept
    {
        static_assert(std::is_invocable_r_v<
            Ret,
            decltype(Function),
            std::tuple_element_t<Index, std::tuple<Args...>>...>);
        data_ = nullptr;

        func_ = [](const void*, std::tuple<Args&& ...> args) -> Ret {
            // Ret(...) makes void(...) eat the return values to avoid errors
            return Ret(std::invoke(Function,
                std::forward<std::tuple_element_t<Index, std::tuple<Args...>>>(std::get<Index>(args))...));
        };
    }

    template<auto Candidate, typename Type, std::size_t... Index>
    void connect(Type& value_or_instance, std::index_sequence<Index...>) noexcept
    {
        static_assert(std::is_invocable_r_v<Ret,
                                            decltype(Candidate), Type&,
                                            std::tuple_element_t<Index, std::tuple<Args...>>...>);
        data_ = &value_or_instance;

        func_ = [](const void* payload, std::tuple<Args&& ...> args) -> Ret {
            Type* curr = static_cast<Type*>(const_cast<std::conditional_t<std::is_const_v<Type>,
                                                                          const void*,
                                                                          void*>>(payload));
            // Ret(...) makes void(...) eat the return values to avoid errors
            return Ret(std::invoke(Candidate, *curr,
                std::forward<std::tuple_element_t<Index, std::tuple<Args...>>>(std::get<Index>(args))...));
        };
    }

    template<auto Candidate, typename Type, std::size_t... Index>
    void connect(Type* value_or_instance, std::index_sequence<Index...>) noexcept
    {
        static_assert(std::is_invocable_r_v<Ret,
                                            decltype(Candidate),
                                            Type*,
                                            std::tuple_element_t<Index, std::tuple<Args...>>...>);
        data_ = value_or_instance;

        func_ = [](const void* payload, std::tuple<Args&& ...> args) -> Ret {
            Type* curr = static_cast<Type*>(const_cast<std::conditional_t<std::is_const_v<Type>,
                                                                          const void*,
                                                                          void*>>(payload));
            // Ret(...) makes void(...) eat the return values to avoid errors
            return Ret(std::invoke(Candidate, curr,
                std::forward<std::tuple_element_t<Index, std::tuple<Args...>>>(std::get<Index>(args))...));
        };
    }

public:
    using function_type = Ret(Args...);

    delegate() noexcept
        : func_{nullptr}, data_{nullptr} {}

    /**
     * @brief Constructs a delegate and connects a free function to it.
     * @tparam Function A valid free function pointer.
     */
    template<auto Function>
    delegate(connect_arg_t<Function>) noexcept
        : delegate{}
    {
        connect<Function>();
    }

    /**
     * @brief Constructs a delegate and connects a member for a given instance
     * or a free function with payload.
     * @tparam Candidate Member or free function to connect to the delegate.
     * @tparam Type Type of class or type of payload.
     * @param value_or_instance A valid reference that fits the purpose.
     */
    template<auto Candidate, typename Type>
    delegate(connect_arg_t<Candidate>, Type& value_or_instance) noexcept
        : delegate{}
    {
        connect<Candidate>(value_or_instance);
    }

    /**
     * @brief Constructs a delegate and connects a member for a given instance
     * or a free function with payload.
     * @tparam Candidate Member or free function to connect to the delegate.
     * @tparam Type Type of class or type of payload.
     * @param value_or_instance A valid pointer that fits the purpose.
     */
    template<auto Candidate, typename Type>
    delegate(connect_arg_t<Candidate>, Type* value_or_instance) noexcept
        : delegate{}
    {
        connect<Candidate>(value_or_instance);
    }

    /**
     * @brief Connects a free function to a delegate.
     * @tparam Function A valid free function pointer.
     */
    template<auto Function>
    void connect() noexcept
    {
        constexpr auto extent =
            internal::function_extent_v<internal::to_function_pointer_t<decltype(Function)>>;
        connect<Function>(std::make_index_sequence<extent>{});
    }

    /**
     * @brief Connects a member function for a given instance or a free function
     * with payload to a delegate.
     *
     * The delegate isn't responsible for the connected object or the payload.
     * Users must always guarantee that the lifetime of the instance overcomes
     * the one  of the delegate.<br/>
     * When used to connect a free function with payload, its signature must be
     * such that the instance is the first argument before the ones used to
     * define the delegate itself.
     *
     * @tparam Candidate Member or free function to connect to the delegate.
     * @tparam Type Type of class or type of payload.
     * @param value_or_instance A valid reference that fits the purpose.
     */
    template<auto Candidate, typename Type>
    void connect(Type& value_or_instance) noexcept
    {
        constexpr auto extent =
            internal::function_extent_v<internal::to_function_pointer_t<decltype(Candidate), Type*>>;
        connect<Candidate>(value_or_instance, std::make_index_sequence<extent>{});
    }

    /**
     * @brief Connects a member function for a given instance or a free function
     * with payload to a delegate.
     *
     * The delegate isn't responsible for the connected object or the payload.
     * Users must always guarantee that the lifetime of the instance overcomes
     * the one  of the delegate.<br/>
     * When used to connect a free function with payload, its signature must be
     * such that the instance is the first argument before the ones used to
     * define the delegate itself.
     *
     * @tparam Candidate Member or free function to connect to the delegate.
     * @tparam Type Type of class or type of payload.
     * @param value_or_instance A valid pointer that fits the purpose.
     */
    template<auto Candidate, typename Type>
    void connect(Type* value_or_instance) noexcept
    {
        constexpr auto extent =
            internal::function_extent_v<internal::to_function_pointer_t<decltype(Candidate), Type*>>;
        connect<Candidate>(value_or_instance, std::make_index_sequence<extent>{});
    }

    /**
     * @brief Resets a delegate.
     *
     * After a reset, a delegate cannot be invoked anymore.
     */
    void reset() noexcept
    {
        func_ = nullptr;
        data_ = nullptr;
    }

    /**
     * @brief Returns the instance or the payload linked to a delegate, if any.
     * @return An opaque pointer to the underlying data.
     */
    const void* instance() const noexcept
    {
        return data_;
    }

    /**
     * @brief Triggers a delegate.
     *
     * The delegate invokes the underlying function and returns the result.
     *
     * @warning
     * Attempting to trigger an invalid delegate results in undefined
     * behavior.<br/>
     * An assertion will abort the execution at runtime in debug mode if the
     * delegate has not yet been set.
     *
     * @param args Arguments to use to invoke the underlying function.
     * @return The value returned by the underlying function.
     */
    Ret operator()(Args... args) const noexcept
    {
        return func_(data_, std::forward_as_tuple(std::forward<Args>(args)...));
    }

    /**
     * @brief Checks whether a delegate actually stores a listener.
     * @return False if the delegate is empty, true otherwise.
     */
    explicit operator bool() const noexcept
    {
        // no need to test also data
        return func_;
    }

    /**
     * @brief Compares the contents of two delegates.
     * @param other Delegate with which to compare.
     * @return False if the two contents differ, true otherwise.
     */
    bool operator==(const delegate<Ret(Args...)>& other) const noexcept
    {
        return func_ == other.func_ && data_ == other.data_;
    }

private:
    proto_fn_type* func_;
    const void* data_;
};

/**
 * @brief Compares the contents of two delegates.
 * @tparam Ret Return type of a function type.
 * @tparam Args Types of arguments of a function type.
 * @param lhs A valid delegate object.
 * @param rhs A valid delegate object.
 * @return True if the two contents differ, false otherwise.
 */
template<typename Ret, typename... Args>
bool operator!=(const delegate<Ret(Args...)>& lhs, const delegate<Ret(Args...)>& rhs) noexcept
{
    return !(lhs == rhs);
}


/**
 * @brief Deduction guide.
 *
 * It allows to deduce the function type of the delegate directly from a
 * function provided to the constructor.
 *
 * @tparam Function A valid free function pointer.
 */
template<auto Function>
delegate(connect_arg_t<Function>) noexcept
-> delegate<std::remove_pointer_t<internal::to_function_pointer_t<decltype(Function)>>>;


/**
 * @brief Deduction guide.
 *
 * It allows to deduce the function type of the delegate directly from a member
 * or a free function with payload provided to the constructor.
 *
 * @tparam Candidate Member or free function to connect to the delegate.
 * @tparam Type Type of class or type of payload.
 */
template<auto Candidate, typename Type>
delegate(connect_arg_t<Candidate>, Type&) noexcept
-> delegate<std::remove_pointer_t<internal::to_function_pointer_t<decltype(Candidate), Type*>>>;


/**
 * @brief Deduction guide.
 *
 * It allows to deduce the function type of the delegate directly from a member
 * or a free function with payload provided to the constructor.
 *
 * @tparam Candidate Member or free function to connect to the delegate.
 * @tparam Type Type of class or type of payload.
 */
template<auto Candidate, typename Type>
delegate(connect_arg_t<Candidate>, Type*) noexcept
-> delegate<std::remove_pointer_t<internal::to_function_pointer_t<decltype(Candidate), Type*>>>;

} // namespace gameboy

#endif //GAMEBOY_DELEGATE_H
