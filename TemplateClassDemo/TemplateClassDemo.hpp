#pragma once

#include <iostream>
#include <string>
#include <tuple>
#include <type_traits>

// 1. 优化 process 函数
template<typename... Args>
auto process(Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        return 0;
    }
    else if constexpr ((std::is_integral_v<std::remove_cvref_t<Args>> && ...))
    {
        std::cout << "all args are integral\n";
        return (... + args);
    }
    else
    {
        std::cout << "not all args are int\n";
        return 0;
    }
}

// 2. 使用折叠表达式简化 sum 函数
template<typename... Args>
auto sum(Args... args)
{
    return (... + args);
}

// 3. 使用 C++17 的 inline 变量简化 is_any_of
template<typename T, typename... Types>
inline constexpr bool is_any_of_v = (std::is_same_v<T, Types> || ...);

// 4. 使用 C++20 的 concepts 简化 tuple_for_each
template<typename T>
concept Tuple = requires(T t) { std::tuple_size<std::remove_cvref_t<T>>::value; };

template<Tuple T, typename Func>
void tuple_for_each(T&& t, Func&& f)
{
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (f(std::get<Is>(std::forward<T>(t))), ...);
    }(std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<T>>>{});
}

// 5. CRTP 示例：静态多态性的 Printer
template<typename Derived>
class Printer
{
public:
    void print() const
    {
        static_cast<const Derived*>(this)->print_impl();
    }
};

class IntPrinter : public Printer<IntPrinter>
{
public:
    void print_impl() const
    {
        std::cout << "Printing an integer\n";
    }
};

class StringPrinter : public Printer<StringPrinter>
{
public:
    void print_impl() const
    {
        std::cout << "Printing a string\n";
    }
};

// 6. 可变参数模板 CRTP 示例：混合功能
template<typename Derived, typename... Mixins>
class MixinBase : public Mixins...
{
public:
    void execute()
    {
        static_cast<Derived*>(this)->execute_impl();
    }
};

template<typename... Mixins>
class Processor : public MixinBase<Processor<Mixins...>, Mixins...>
{
public:
    void execute_impl()
    {
        (Mixins::process(), ...);
    }
};

struct LogMixin
{
    void process()
    {
        std::cout << "Logging\n";
    }
};

struct ValidateMixin
{
    void process()
    {
        std::cout << "Validating\n";
    }
};

// 7. Type traits 示例：检查是否所有类型都是整数
template<typename... Ts>
struct are_all_integral : std::conjunction<std::is_integral<Ts>...>
{
};

template<typename... Ts>
inline constexpr bool are_all_integral_v = are_all_integral<Ts...>::value;
