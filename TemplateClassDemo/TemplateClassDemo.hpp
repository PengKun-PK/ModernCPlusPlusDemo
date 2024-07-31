#pragma once

#include <iostream>
#include <string>

template<typename... Args>
int process(Args&&... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        return 0;
    }
    else if constexpr ((std::is_integral_v<std::remove_reference_t<Args>> && ...))
    {
        std::cout << "all args are integral \n";
        return (0 + ... + args);
    }
    else
    {
        std::cout << "not all args are int \n";
        return 0;
    }
}

template<typename... Args>
int sum(Args... args)
{
    if constexpr (sizeof...(Args) == 0)
    {
        return 0;
    }
    else
    {
        return (... + args);
    }
}

// 主模板声明
template<typename T, typename... Types>
struct is_any_of : std::false_type
{
};
// 特化版本,用于递归处理类型列表
template<typename T, typename First, typename... Rest>
struct is_any_of<T, First, Rest...>
    : std::conditional_t<std::is_same_v<T, First>, std::true_type, is_any_of<T, Rest...>>
{
};

// 主函数模板
template<typename Tuple, typename Func, std::size_t... Is>
void tuple_for_each_impl(Tuple&& t, Func&& f, std::index_sequence<Is...>)
{
    (f(std::get<Is>(std::forward<Tuple>(t))), ...);
}

// 用户接口函数
template<typename Tuple, typename Func>
void tuple_for_each(Tuple&& t, Func&& f)
{
    tuple_for_each_impl(std::forward<Tuple>(t), std::forward<Func>(f),
                        std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
}
