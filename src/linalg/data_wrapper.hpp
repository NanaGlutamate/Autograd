#pragma once

#include <cstdint>
#include <type_traits>

#include "meta_helper.hpp"

namespace autograd {

template <typename T>
struct Identity;

template <typename T>
struct Zero;

// template <typename T>
// struct Ones;

template <typename T>
struct is_commutative { inline static constexpr bool value = false; };

template <typename T>
    requires std::is_arithmetic_v<T>
struct is_commutative<T> { inline static constexpr bool value = true; };

template <typename T>
concept is_commutative_v = is_commutative<T>::value;

template <typename T>
    requires std::is_arithmetic_v<T>
struct Identity<T> {
    inline static constexpr T value = 1;
};

template <typename T>
    requires std::is_arithmetic_v<T>
struct Zero<T> {
    inline static constexpr T value = 0;
};

// template <typename T>
//     requires std::is_arithmetic_v<T>
// struct Ones<T> {
//     inline static constexpr T value = 1;
// };

template <typename T>
concept Group = requires(T a) { Zero<T>::value; };

template <typename T>
concept Ring = requires(T a) { Identity<T>::value; Zero<T>::value; };

}