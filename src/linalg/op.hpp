#pragma once

#include "tensor.hpp"
#include <cstring>
#include <sys/cdefs.h>
#include <type_traits>

namespace linalg {

namespace impl {

template <typename T>
void gemm_ab(
    __restrict T* A, __restrict T* B, __restrict T* C,
    size_t I, size_t J, size_t K
) {
    std::memset(C, 0, I * K);
    for (size_t i = 0; i < I; ++i) {
        for (size_t j = 0; j < J; ++j) {
            T rate = A[i * J + j];
            for (size_t k = 0; k < K; ++k) {
                C[i * K + k] += rate * B[j * K + k];
            }
        }
    }
}

template <typename T>
void gemm_atb(
    __restrict T* A, __restrict T* B, __restrict T* C,
    size_t I, size_t J, size_t K
) {
}

template <typename T>
void gemm_abt(
    __restrict T* A, __restrict T* B, __restrict T* C,
    size_t I, size_t J, size_t K
) {
    for (size_t i = 0; i < I; ++i) {
        for (size_t k = 0; k < K; ++k) {
            T temp = 0;
            for (size_t j = 0; j < J; ++j) {
                temp += A[i * J + j] * B[k * J + j];
            }
            C[i * K + k] = temp;
        }
    }
}

template <typename T>
void gemm_atbt(
    __restrict T* A, __restrict T* B, __restrict T* C,
    size_t I, size_t J, size_t K
) {
}

template <typename T>
void gemm(
    __restrict T* A, __restrict T* B, __restrict T* C,
    size_t I, size_t J, size_t K,
    bool ATrans, bool BTrans
) {
    if (!ATrans && !BTrans) {
        gemm_ab(A, B, C, I, J, K);
    } else if (!ATrans && BTrans) {
        gemm_abt(A, B, C, I, J, K);
    } else if (ATrans && !BTrans) {
        gemm_atb(A, B, C, I, J, K);
    } else {
        gemm_atbt(A, B, C, I, J, K);
    }
}

template <typename T, size_t I, size_t J, size_t K, bool ATrans, bool BTrans>
void gemm(T* A, T* B, T* C) {
    gemm<T>(A, B, C, I, J, K, ATrans, BTrans);
}

}

template <typename T, typename V, size_t L, typename F>
    requires std::is_same_v<V, std::invoke_result_t<F, V>>
void map(T* src, V* dst, F&& f) {
    for (size_t i = 0; i < L; ++i) {
        dst[i] = f(src[i]);
    }
}

}