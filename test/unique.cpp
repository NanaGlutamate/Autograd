#include <memory>
#include <utility>
#include <iostream>
#include <format>
#include <vector>

#include "linalg/forward.hpp"
#include "unique_ref.hpp"
#include "dense_vector.hpp"
#include "mimalloc-new-delete.h"
#include "linalg/backward.hpp"

// struct A {
//     int a;
// };

// template <typename T>
// int&& test(T&& a) {
//     return std::forward<T>(a).a;
// }

// decltype(auto) test2(auto&& a) {
//     return std::forward_like<decltype(a)>(test(std::move(a)));
// }

// inline constexpr int x = 1;
// const int& test3() {
//     return x;
// }

// struct A {
//     ~A() { std::cout << "A" << std::endl; }
// };

// struct B : public A {
//     ~B() { std::cout << "B" << std::endl; }
// };

// template <typename T>
// struct holder { T data; };
// template <typename T>
// holder(T) -> holder<T>;

// struct A {
//     int a;
//     int b;
//     A(int, int) {}
// };

// struct B : public A {
//     int b;
//     void foo() {}
// };

// void (A::*p)() = &B::foo;

template <typename T>
struct A{};

int main() {
    using namespace autograd;
    using namespace graph;
    autograd::BackwardGradWrapper<int, true> x{2};
    autograd::BackwardGradWrapper<int, true> y{3};
    auto t1 = 3 + x;
    auto t2 = t1 + y;
    auto t3 = t2 + 12;
    // x += autograd::BackwardGradWrapper<int>{3, true};
    // x += autograd::BackwardGradWrapper<int>{4, false};
    t3.backward();
    std::cout << *x.grad();
    // autograd::ForwardGradWrapper<float> x{1.0f, true};
    // constexpr bool isop = autograd::traits::can_operate<int, int, autograd::ForwardGradWrapper<int>>;
    // auto fx = x + 2.0f;
    // A a{0};
    // decltype(a) b;
    // test2(std::move(a));
    return 0;
}