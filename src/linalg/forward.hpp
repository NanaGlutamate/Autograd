// #pragma once

// #include "linalg/data_wrapper.hpp"
// #include <functional>
// #include <type_traits>
// #include <memory>
// #include <utility>
// #include <unordered_map>
// #include <ranges>

// namespace autograd {

// template <is_commutative_v T, typename Allocator = std::allocator<T>>
// struct ForwardGradWrapper;

// namespace traits {

// template <typename T>
// inline constexpr bool is_forward_grad_wrapper_v = false;

// template <typename T>
//     requires std::is_same_v<T, std::remove_cvref_t<T>>
// inline constexpr bool is_forward_grad_wrapper_v<ForwardGradWrapper<T>> = true;


// template <typename T>
// struct param_data_type { using type = T; };

// template <typename T>
//     requires is_forward_grad_wrapper_v<std::remove_cvref_t<T>>
// struct param_data_type<T> { using type = std::remove_cvref_t<T>::DataType; };

// template <typename T>
// using param_data_type_t = param_data_type<T>::type;


// template <typename TargetDataType, typename ...Args>
// concept can_operate = ((is_forward_grad_wrapper_v<std::remove_cvref_t<Args>> &&
//         std::is_constructible_v<TargetDataType, param_data_type_t<Args>> || std::is_constructible_v<TargetDataType, Args>) && ...);

// }

// template <typename T>
// constexpr bool param_requires_grad(T&& data) {
//     if constexpr (traits::is_forward_grad_wrapper_v<std::remove_cvref_t<T>>) {
//         return data.requires_grad;
//     } else {
//         return false;
//     }
// }

// template <typename T>
// decltype(auto) param_unwrap(T&& data) {
//     if constexpr (traits::is_forward_grad_wrapper_v<std::remove_cvref_t<T>>) {
//         return data.getDataRef();
//     } else {
//         return std::forward<T>(data);
//     }
// }

// /**
//  * @brief a wrapper to provide forward gradient calculation ability for multiple data (scala, vector or matrix)
//  * 
//  * we should store \frac{\partial y}{\partial x} in a homogeneity container,
//  * so any type involved in calculation should be the same (unless donot need to store it's diff,
//  * in which case it should not passed as forward_grad_wrapper<T>, but passed as T barely)
//  * 
//  * @tparam T inner data type
//  */
// template <is_commutative_v T, typename Allocator>
//     requires std::is_same_v<T, std::remove_cvref_t<T>>
// struct ForwardGradWrapper<T, Allocator> {
//     // todo: allocator
//     std::unique_ptr<T> data;

//     using DataType = T;

//     template <typename Args>
//     constexpr ForwardGradWrapper(
//         Args&& data,
//         bool requires_grad = false
//     ): data(std::make_unique(std::forward<Args>(data))), diff(), requires_grad(requires_grad) {
//         if (requires_grad) {
//             diff[data.get()] = {};
//         }
//     }
//     constexpr ForwardGradWrapper(
//         const ForwardGradWrapper& other
//     ): data(std::make_unique(*other.data)), diff(other.diff), requires_grad(other.requires_grad) {}
//     constexpr ForwardGradWrapper(ForwardGradWrapper&& other) noexcept = default;
//     constexpr ForwardGradWrapper& operator=(const ForwardGradWrapper& other) {
//         if (this == &other) return *this;
//         data = std::make_unique(*other.data);
//         diff = other.diff;
//         requires_grad = other.requires_grad;
//         return *this;
//     }
//     constexpr ForwardGradWrapper& operator=(ForwardGradWrapper&& other) noexcept = default;
//     constexpr ~ForwardGradWrapper() = default;

//     template <typename Self>
//     decltype(auto) getDataRef(this Self&& self) { return std::forward_like<Self>(*self.data); }
//     template <typename Self>
//     decltype(auto) operator->(this Self&& self) { return &std::forward_like<Self>(*self.data); }

//     // a.partial_to(a); // assert 1, expect nullptr
//     constexpr const T& partial_to(const ForwardGradWrapper& other) const requires Group<T> {
//         if (auto it = diff.find(other.data.get()); it != diff.end()) {
//             return &it->second;
//         } else {
//             // fixme: ref to temp data?
//             return Zero<T>::value;
//         }
//     }
//     // // todo:
//     // void stop_trace_except(std::ranges::range auto data) {}

//     template <typename F, typename Fd, typename ...U>
//         requires (std::is_same_v<T, traits::param_data_type_t<U>> && ...)
//                 && std::is_constructible_v<T, std::invoke_result<F, U...>>
//     constexpr static auto transform(F&& func, Fd&& diff_func, U&&... inputs) {
//         auto unwrapData = []<typename D>(D&& d) -> decltype(auto) {
//             if constexpr (traits::is_forward_grad_wrapper_v<decltype(d)>) {
//                 return std::forward_like<D>(*d.data);
//             } else {
//                 return std::forward<D>(d);
//             }
//         };
//         auto unwrapDiff = []<typename D>(D&& d) -> decltype(auto) {
//             const static std::unordered_map<T*, T> empty_diff{};
//             if constexpr (traits::is_forward_grad_wrapper_v<decltype(d)>) {
//                 return std::forward_like<D>(d.diff);
//             } else {
//                 return (empty_diff);
//             }
//         };
//         ForwardGradWrapper<T> ans {std::invoke(std::forward<F>(func), unwrapData(std::forward<U>(inputs))...)};
//         ans.diff = std::invoke(std::forward<Fd>(diff_func), unwrapDiff(std::forward<U>(inputs))...);
//         return ans;
//     }

//     template <typename A, typename B>
//         requires traits::can_operate<T, A, B> 
//                 && requires(traits::param_data_type_t<A> a, traits::param_data_type_t<B> b) { a + b; }
//     constexpr friend auto operator+(A&& a, B&& b) {
//         return transform(
//             [](auto&& x, auto&& y) { return x + y; },
//             [](auto&& dx, auto&& dy) {
//                 std::unordered_map<T*, T> diff{};
//                 auto merge_diff = [&diff]<typename Diff>(Diff&& d) {
//                     for (auto&& [k, v] : d) {
//                         if (auto it = diff.find(k); it != diff.end()) {
//                             it->second += v;
//                         } else {
//                             diff.emplace(k, std::forward_like<Diff>(v));
//                         }
//                     }
//                 };
//                 merge_diff(std::forward<decltype(dx)>(dx));
//                 merge_diff(std::forward<decltype(dy)>(dy));
//                 return diff;
//             },
//             std::forward<A>(a), std::forward<B>(b)
//         );
//     }

// private:
//     struct Differential {
//         T left = Identity<T>::value, right = Identity<T>::value;
//     };
//     // todo: left mul and right mul
//     std::unordered_map<T*, Differential> diff;
//     bool requires_grad;
// };

// }