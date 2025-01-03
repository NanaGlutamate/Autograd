#pragma once

#ifndef __SRC_DENSE_VECTOR_HPP__
#define __SRC_DENSE_VECTOR_HPP__

#include <utility>
#include <assert.h>

template<typename T, size_t Capacity>
struct dense_vector {
    dense_vector() {}
    size_t size() { return curr_size; }
    T* begin() { return static_cast<T*>(data); }
    T* end() { return static_cast<T*>(data) + curr_size; }
    template <typename ...Args>
    T* emplace_back(Args&&... args) {
        assert(curr_size != Capacity && "call emplace on full vector");
        auto p = end();
        new (p) T {std::forward<Args>(args)...};
        curr_size++;
        return p;
    }
    void pop_back() {
        assert(curr_size != 0 && "call pop on empty vector");
        (end() - 1)->~T();
        curr_size--;
    }
private:
    size_t curr_size;
    alignas(T) std::byte data[Capacity * sizeof(T)];
};

#endif