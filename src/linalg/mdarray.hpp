#pragma once

namespace linalg {

template <typename T, size_t ...DIM>
    requires ((DIM > 0) && ...)
struct MDArray {
    T data[(DIM * ... * 1)];
};

template <typename T, size_t ...DIM>
    requires ((DIM == -1 || DIM > 0) && ...) && ((DIM == -1) || ...)
struct MDArray {
    T* data;
};

}
