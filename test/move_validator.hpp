#pragma once

#include <cstddef>
#include <utility>

template <typename T>
struct MoveValidator {
    inline static struct Context {
        inline constexpr static size_t IGNORE = -1;

        size_t construct = 0;
        size_t move_construct = 0;
        size_t copy_construct = 0;
        size_t move_assignment = 0;
        size_t copy_assignment = 0;
        size_t destruct = 0;
        size_t self_assign = 0;

        Context operator-(const Context& other) const {
            return Context{
                construct - other.construct,
                move_construct - other.move_construct, 
                copy_construct - other.copy_construct,
                move_assignment - other.move_assignment,
                copy_assignment - other.copy_assignment,
                destruct - other.destruct,
                self_assign - other.self_assign
            };
        }

        static bool member_equal(size_t a, size_t b) {
            return a == IGNORE || b == IGNORE || a == b;
        }

        bool operator==(const Context& other) const {
            return member_equal(construct, other.construct) &&
                    member_equal(move_construct, other.move_construct) &&
                    member_equal(copy_construct, other.copy_construct) &&
                    member_equal(move_assignment, other.move_assignment) &&
                    member_equal(copy_assignment, other.copy_assignment) &&
                    member_equal(destruct, other.destruct) &&
                    member_equal(self_assign, other.self_assign);
        }
    } ctx {};

    explicit MoveValidator(T t) : data(std::move(t)) { ++ctx.construct; }
    ~MoveValidator() { ++ctx.destruct; }
    MoveValidator(const MoveValidator& other) : data(other.data) { ++ctx.copy_construct; }
    MoveValidator(MoveValidator&& other) : data(std::move(other.data)) { ++ctx.move_construct; }
    MoveValidator& operator=(const MoveValidator& other) {
        if (this == &other) {
            ++ctx.self_assign;
            return *this;
        }
        data = other.data;
        ++ctx.copy_assignment;
        return *this;
    }
    MoveValidator& operator=(MoveValidator&& other) {
        if (this == &other) {
            ++ctx.self_assign;
            return *this;
        }
        data = std::move(other.data);
        ++ctx.move_assignment;
        return *this;
    }
    T data;
};