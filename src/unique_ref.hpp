#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#ifndef __SRC_SHARED_UNIQUE_HPP__
#define __SRC_SHARED_UNIQUE_HPP__

#include <atomic>
#include <memory>

template <typename T, typename Allocator>
struct weak_ref;

template <typename T, typename Allocator = std::allocator<T>>
struct unique_ref : private Allocator {
    friend struct weak_ref<T, Allocator>;
    template <typename ...Args>
    explicit unique_ref(Args&&... args) : handler() {
        handler.data = handler.allocate(1);
        handler.data = std::construct_at(handler.data, std::forward<Args>(args)...);
    }
    unique_ref(const unique_ref&) = delete;
    unique_ref(unique_ref&& other) noexcept : handler(other.handler) {
        other.handler.data = nullptr;
    }
    unique_ref& operator=(const unique_ref&) = delete;
    unique_ref& operator=(unique_ref&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        handler.dec_cnt();
        handler = other.handler;
        other.handler = nullptr;
        return *this;
    }
    ~unique_ref() {
        handler.expired(this);
        handler.dec_cnt();
    }

    T* operator->() { return &(handler.data->obj); }

private:
    struct mem {
        std::atomic<size_t> cnt;
        std::atomic<T*> ptr;
        T obj;
        template <typename ...Args>
        mem(Args&&...args) : cnt(1), ptr(&obj), obj(std::forward<Args>(args)...) {}
    };
    struct Ptr : public std::allocator_traits<Allocator>::template rebind_alloc<mem> {
        mem* data;
        void inc_cnt() noexcept {
            if (!data) {
                return;
            }

            data->cnt.fetch_add(1, std::memory_order_seq_cst);
        }
        void expired(Allocator* allocator) noexcept {
            if (!data) {
                return;
            }

            T* target = data->ptr.exchange(nullptr, std::memory_order_release);
            std::allocator_traits<Allocator>::destroy(*allocator, target);
        }
        void dec_cnt() noexcept {
            if (!data) {
                return;
            }

            size_t cnt = data->cnt.fetch_sub(1, std::memory_order_seq_cst);
            if (cnt == 0) {
                static_assert(std::is_trivially_destructible<std::atomic<size_t>>::value);
                static_assert(std::is_trivially_destructible<std::atomic<T*>>::value);
                this->deallocate(data, 1);
                data = nullptr;
            }
        }
    } handler;
};

template <typename T, typename Allocator = std::allocator<T>>
struct weak_ref {
    explicit weak_ref(const unique_ref<T, Allocator>& other) : handler(other.handler) {
        handler.inc_cnt();
    }
    weak_ref(const weak_ref& other) : handler(other.handler) {
        handler.inc_cnt();
    }
    weak_ref(weak_ref&& other) noexcept : handler(other.handler) {
        other.handler = nullptr;
    }
    weak_ref& operator=(const weak_ref& other) {
        if (this == &other) {
            return *this;
        }
        handler.dec_cnt();
        handler = other.handler;
        handler.inc_cnt();
        return *this;
    }
    weak_ref& operator=(weak_ref&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        handler.dec_cnt();
        handler = other.handler;
        other.handler = nullptr;
        return *this;
    }
    bool is_expired() const {
        return handler.data == nullptr || handler.data->ptr.load(std::memory_order_consume) == nullptr;
    }
private:
    unique_ref<T, Allocator>::Ptr handler;
};

template <typename T, typename Allocator = std::allocator<T>>
weak_ref(const unique_ref<T, Allocator>&) -> weak_ref<T, Allocator>;

#endif