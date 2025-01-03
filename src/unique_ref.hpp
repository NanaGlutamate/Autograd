// #pragma once

// #ifndef __SRC_UNIQUE_REF_HPP__
// #define __SRC_UNIQUE_REF_HPP__

// #include <cstddef>
// #include <type_traits>
// #include <utility>
// #include <atomic>
// #include <memory>

// template <typename T, typename Allocator>
// struct weak_ref;

// template <typename T, typename Allocator = std::allocator<T>>
// struct unique_ref : private Allocator {
//     friend struct weak_ref<T, Allocator>;
//     template <typename ...Args>
//     explicit unique_ref(Args&&... args) : handler(std::forward<Args>(args)...) {}
//     explicit unique_ref(T* data) : handler(data) {}
//     unique_ref(const unique_ref&) = delete;
//     unique_ref(unique_ref&& other) noexcept = default;
//     unique_ref& operator=(const unique_ref&) = delete;
//     unique_ref& operator=(unique_ref&& other) noexcept = default;
//     ~unique_ref() {
//         handler.release(*this);
//     };

//     T* get() noexcept { return handler.cb->ptr.load(); }
//     T* operator->() noexcept { return get(); }
//     bool observered() noexcept { return handler.cb->cnt.load() != 1; }

// private:
//     struct ControlBlock {
//         ControlBlock(size_t cnt, T* ptr, bool merged = false): cnt(cnt), ptr(ptr), merged(merged) {}
//         bool merged;
//     private:
//         std::atomic<size_t> cnt;
//         std::atomic<T*> ptr;
//         static_assert(std::atomic<size_t>::is_always_lock_free);
//         static_assert(std::atomic<T*>::is_always_lock_free);
//     };
//     struct MergedControlBlock : public ControlBlock {
//         T obj;
//         template <typename ...Args>
//         MergedControlBlock(Args&&...args) : ControlBlock(1, &obj, true), obj(std::forward<Args>(args)...) {}
//     };
//     static_assert(std::is_trivially_destructible_v<ControlBlock>);
//     static_assert(std::is_trivially_destructible_v<MergedControlBlock>);

//     struct Handler : private std::allocator_traits<Allocator>::template rebind_alloc<MergedControlBlock>,
//         private std::allocator_traits<Allocator>::template rebind_alloc<ControlBlock> {
//         using MergedSuper = std::allocator_traits<Allocator>::template rebind_alloc<MergedControlBlock>;
//         using Super = std::allocator_traits<Allocator>::template rebind_alloc<ControlBlock>;
//         Handler(T* data) {
//             cb = new(Super::allocate(1)) ControlBlock {1, data};
//         }
//         template<typename ...Args>
//             requires std::is_constructible_v<T, Args...>
//         Handler(Args&&... args) {
//             cb = new(MergedSuper::allocate(1)) ControlBlock {std::forward<Args>(args)...};
//         }
//         Handler(const Handler& other) noexcept {
//             cb = other.cb;
//             cb->cnt.fetch_add(1);
//         }
//         Handler(Handler&& other) noexcept {
//             cb = other.cb;
//             other.cb = nullptr;
//         }
//         Handler& operator=(const Handler& other) noexcept {
//             if (this == &other) {
//                 return *this;
//             }
//             cb = other.cb;
//             cb->cnt.fetch_add(1);
//         }
//         Handler& operator=(Handler&& other) noexcept {
//             if (this == &other) {
//                 return *this;
//             }
//             cb = other.cb;
//             other.cb = nullptr;
//         }
//         void release(unique_ref& holder) {
//             T* temp_data = nullptr;
//             cb->data.swap(temp_data);
//             if (temp_data == nullptr) {
//                 return;
//             }
//             temp_data->~T();
//             if (!cb->merged) {
//                 holder.deallocate(temp_data);
//             }
//         }
//         ~Handler() {
//             if (cb == nullptr) {
//                 return;
//             }
//             size_t after_cnt = cb->cnt.fetch_sub(1);
//             if (after_cnt == 0) {
//                 if (cb->merged) {
//                     MergedSuper::deallocate(cb);
//                 } else {
//                     Super::deallocate(cb);
//                 }
//             }
//         }
//     private:
//         ControlBlock* cb;
//     } handler;
// };

// template <typename T, typename Allocator = std::allocator<T>>
// struct weak_ref {
//     explicit weak_ref(const unique_ref<T, Allocator>& other) : handler(other.handler) {}
//     weak_ref(const weak_ref& other) = default;
//     weak_ref(weak_ref&& other) noexcept = default;
//     weak_ref& operator=(const weak_ref& other) = default;
//     weak_ref& operator=(weak_ref&& other) noexcept = default;
//     bool is_expired() const {
//         return handler.cb->data.load(std::memory_order_relaxed) == nullptr;
//     }
//     T* get() { return handler.cb->data.load(std::memory_order_relaxed); }
// private:
//     unique_ref<T, Allocator>::Handler handler;
// };

// template <typename T, typename Allocator = std::allocator<T>>
// weak_ref(const unique_ref<T, Allocator>&) -> weak_ref<T, Allocator>;

// #endif