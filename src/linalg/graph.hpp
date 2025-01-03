#pragma once

#include <optional>
#include <memory>
#include <type_traits>
#include <utility>
#include <string_view>
#include <typeinfo>

namespace autograd {

namespace graph {

template <typename T>
struct Node {
    T data;

    template <typename Data>
        requires std::is_constructible_v<T, Data>
    explicit Node(Data&& data): data(std::forward<Data>(data)) {}
    Node(const Node&) = delete;
    Node(Node&&) = default;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = default;
    virtual ~Node() = default;

    virtual void call_backward(const T& this_diff, bool keep_graph = false) {}
protected:
    virtual void backward(bool) { /* do nothing */ }
    virtual std::string_view name() const { return ""; };
};

template <typename T, bool is_retain_grad = true>
struct DiffNode : public Node<T> {
    using DiffHolder = std::conditional_t<is_retain_grad, std::optional<T>, std::unique_ptr<T>>;
    DiffHolder diff;

    template <typename Data, typename Diff>
        requires std::is_constructible_v<T, Data>
    explicit DiffNode(Data&& data, Diff&& diff): 
            Node<T>(std::forward<Data>(data)),
            diff(std::forward<Diff>(diff)) {}
    DiffNode(const DiffNode&) = delete;
    DiffNode(DiffNode&&) = default;
    DiffNode& operator=(const DiffNode&) = delete;
    DiffNode& operator=(DiffNode&&) = default;

    void call_backward(const T& this_diff, bool keep_graph = false) override {
        if (!diff.has_value()) {
            diff.emplace(0);
        }
        diff.value() += this_diff;
        this->backward(keep_graph);
    }
    void clear() {
        diff.clear();
    }
};

template <typename T, typename Operations>
    requires std::is_same_v<T, std::remove_cvref_t<T>> && std::is_same_v<Operations, std::remove_cvref_t<Operations>>
struct DiffNodeWrapper final : public DiffNode<T> {
    Operations op;
    template <typename Data, typename Op>
        requires std::is_constructible_v<T, Data> && std::is_invocable_v<Op, DiffNodeWrapper&, bool>
    explicit DiffNodeWrapper(Data&& data, Op&& op):
            DiffNode<T>(std::forward<Data>(data), std::nullopt),
            op(std::forward<Op>(op)) {}
    DiffNodeWrapper(const DiffNodeWrapper&) = delete;
    DiffNodeWrapper(DiffNodeWrapper&&) = default;
    DiffNodeWrapper& operator=(const DiffNodeWrapper&) = delete;
    DiffNodeWrapper& operator=(DiffNodeWrapper&&) = default;

protected:
    void backward(bool keep_graph) override {
        op(*this, keep_graph);
    }
    std::string_view name() const override {
        return typeid(Operations).name();
    }
};
template <typename Data, typename Op>
explicit DiffNodeWrapper(Data&& data, Op&& operations) -> 
        DiffNodeWrapper<std::remove_cvref_t<Data>, std::remove_cvref_t<Op>>;


template <typename T, typename L, typename R, typename Data>
auto add_op(Data&& data, std::shared_ptr<graph::Node<L>> lhs, std::shared_ptr<graph::Node<R>> rhs) {
    DiffNodeWrapper node{
        std::forward<Data>(data),
        [lhs{std::move(lhs)}, rhs{std::move(rhs)}](DiffNode<T>& node, bool keep_graph) mutable {
            if (lhs != nullptr) {
                lhs->call_backward(node.diff.value(), keep_graph);
            }
            if (rhs != nullptr) {
                rhs->call_backward(node.diff.value(), keep_graph);
            }
            if (!keep_graph) {
                lhs.reset();
                rhs.reset();
            }
        }
    };
    return std::make_shared<std::remove_cvref_t<decltype(node)>>(std::move(node));
}

template <typename T, typename L, typename R, typename Data>
auto mul_op(Data&& data, std::shared_ptr<graph::Node<L>> lhs, std::shared_ptr<graph::Node<R>> rhs) {
    DiffNodeWrapper node{
        std::forward<Data>(data),
        [lhs{std::move(lhs)}, rhs{std::move(rhs)}](DiffNode<T>& node, bool keep_graph) mutable {
            if (lhs != nullptr) {
                // fixme: transpose
                lhs->call_backward(node.diff * rhs->data, keep_graph);
            }
            if (rhs != nullptr) {
                // fixme: transpose
                rhs->call_backward(lhs->data * node.diff, keep_graph);
            }
            if (!keep_graph) {
                lhs.reset();
                rhs.reset();
            }
        }
    };
    return std::make_shared<std::remove_cvref_t<decltype(node)>>(std::move(node));
}

}

}