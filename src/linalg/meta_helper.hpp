#pragma once

namespace autograd {

template <template <typename A, typename B> typename F>
struct Curry {
    template <typename T>
    struct result {
        template <typename U>
        using type = F<T, U>;
    };
};

template <typename ...V>
struct TypeList {
    template <template <typename T> typename Func>
    consteval static bool all() { return (Func<V>::value && ...); }

    template <template <typename T> typename Func>
    consteval static bool any() { return (Func<V>::value || ...); }

    template <template <typename T> typename Func>
    using map = TypeList<Func<V>...>;
};

}