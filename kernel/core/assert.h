// kernel/core/assert.h
#ifndef ASSERT_H
#define ASSERT_H

#include "panic.h"

// Static assert for compile-time checks
#define STATIC_ASSERT(condition, message) \
    typedef char static_assertion_##message[(condition)?1:-1]

// Runtime assert
#define KASSERT(expr) \
    if (!(expr)) { \
        PANIC("Assertion failed: " #expr); \
    }

// Verify with message
#define VERIFY(expr, msg) \
    if (!(expr)) { \
        PANIC(msg); \
    }

#endif // ASSERT_H