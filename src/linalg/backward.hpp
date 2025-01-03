#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include "graph.hpp"

namespace autograd {

// todo: support no-retain grad
template <typename T, bool is_requires_grad, bool is_retain_grad = is_requires_grad>
struct BackwardGradWrapper;

namespace traits {

template <typename T>
inline constexpr bool is_backward_grad_wrapper_v = false;

template <typename T, bool is_requires_grad, bool is_retain_grad>
    requires std::is_same_v<T, std::remove_cvref_t<T>>
inline constexpr bool is_backward_grad_wrapper_v<BackwardGradWrapper<T, is_requires_grad, is_retain_grad>> = true;


template <typename T>
inline constexpr bool is_requires_grad_v = false;

template <typename T, bool is_requires_grad, bool is_retain_grad>
    requires std::is_same_v<T, std::remove_cvref_t<T>>
inline constexpr bool is_requires_grad_v<BackwardGradWrapper<T, is_requires_grad, is_retain_grad>> = is_requires_grad;

template <typename T>
inline constexpr bool is_requires_grad_no_cvref_v = is_requires_grad_v<std::remove_cvref_t<T>>;


template <typename T>
inline constexpr bool is_retain_grad_v = false;

template <typename T, bool is_requires_grad, bool is_retain_grad>
    requires std::is_same_v<T, std::remove_cvref_t<T>>
inline constexpr bool is_retain_grad_v<BackwardGradWrapper<T, is_requires_grad, is_retain_grad>> = is_retain_grad;

template <typename T>
inline constexpr bool is_retain_grad_no_cvref_v = is_retain_grad_v<std::remove_cvref_t<T>>;


template <typename T>
struct param_data_type { using type = std::remove_cvref_t<T>; };

template <typename T>
    requires is_backward_grad_wrapper_v<std::remove_cvref_t<T>>
struct param_data_type<T> { using type = std::remove_cvref_t<T>::DataType; };

template <typename T>
using param_data_type_t = param_data_type<T>::type;


template <typename TargetDataType, typename ...Args>
concept can_operate = ((
        (is_backward_grad_wrapper_v<std::remove_cvref_t<Args>> 
            && std::is_constructible_v<TargetDataType, param_data_type_t<Args>>) 
        || std::is_constructible_v<TargetDataType, Args>) && ...);

}

template <typename T, bool is_requires_grad, bool is_retain_grad>
struct BackwardGradWrapper final {
    inline static constexpr bool requires_grad = is_requires_grad;

    BackwardGradWrapper(T data) requires requires_grad: 
            node(std::make_shared<graph::DiffNode<T>>(std::move(data), std::nullopt)) {}
    BackwardGradWrapper(T data) requires (!requires_grad): 
            node(std::make_shared<graph::Node<T>>(std::move(data))) {}
    BackwardGradWrapper(const BackwardGradWrapper& other) = delete;
    BackwardGradWrapper(BackwardGradWrapper&& other) noexcept = default;
    BackwardGradWrapper& operator=(const BackwardGradWrapper& other) = delete;
    BackwardGradWrapper& operator=(BackwardGradWrapper&& other) noexcept = default;
    ~BackwardGradWrapper() = default;

    using DataType = T;

    void backward(bool keep_graph = false) requires std::is_constructible_v<T, int> {
        node->call_backward(1, keep_graph);
    }

    const std::optional<T>& grad() const& requires requires_grad {
        return node->diff;
    }

    template <typename Self, bool new_requires_grad = !is_requires_grad>
    auto as_requires_grad(this Self&& self) {
        if constexpr (Self::requires_grad) {
            return BackwardGradWrapper<T, new_requires_grad>{self.node->data};
        } else {
            return BackwardGradWrapper<T, new_requires_grad>{std::forward_like<Self>(self.node->data)};
        }
    }

    template <typename L, typename R>
    friend auto operator+(L&& lhs, R&& rhs) {
        constexpr bool new_requires_grad = traits::is_requires_grad_v<std::remove_cvref_t<L>> 
                || traits::is_requires_grad_v<std::remove_cvref_t<R>>;
        if constexpr (new_requires_grad) {
            return BackwardGradWrapper<T, true>{
                graph::add_op<T, traits::param_data_type_t<L>, traits::param_data_type_t<R>>(
                    unwrap(lhs) + unwrap(rhs),
                    make_node(lhs),
                    make_node(rhs)
                )
            };
        } else {
            return BackwardGradWrapper<T, false>{
                unwrap(std::forward<L>(lhs)) + unwrap(std::forward<R>(rhs))
            };
        }
    }

    template <typename L, typename R>
    friend auto operator*(L&& lhs, R&& rhs) {
        constexpr bool new_requires_grad = traits::is_requires_grad_v<std::remove_cvref_t<L>> 
                || traits::is_requires_grad_v<std::remove_cvref_t<R>>;
        if constexpr (new_requires_grad) {
            return BackwardGradWrapper<T, true>{
                graph::mul_op<T, traits::param_data_type_t<L>, traits::param_data_type_t<R>>(
                    unwrap(lhs) * unwrap(rhs),
                    make_node(lhs),
                    make_node(rhs)
                )
            };
        } else {
            return BackwardGradWrapper<T, false>{
                unwrap(std::forward<L>(lhs)) * unwrap(std::forward<R>(rhs))
            };
        }
    }

private:
    using NodeType = std::conditional_t<requires_grad, graph::DiffNode<T>, graph::Node<T>>;
    const std::shared_ptr<NodeType> node;

    explicit BackwardGradWrapper(std::shared_ptr<NodeType> node) requires requires_grad: 
            node(std::move(node)) {}

    template <typename Container>
    static constexpr decltype(auto) make_node(Container&& data) {
        if constexpr (traits::is_backward_grad_wrapper_v<std::remove_cvref_t<Container>>) {
            return data.node;
        } else {
            return std::make_shared<graph::Node<std::remove_cvref_t<Container>>>(data);
        }
    }

    template <typename Container>
    static constexpr decltype(auto) unwrap(Container&& data) {
        if constexpr (traits::is_backward_grad_wrapper_v<std::remove_cvref_t<Container>>) {
            return data.node->data;
        } else {
            return std::forward<Container>(data);
        }
    }
};

}