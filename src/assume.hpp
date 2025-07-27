//
// Created by kylan on 3/26/2025.
//

#ifndef ASSUME_H
#define ASSUME_H

#ifdef NDEBUG
#define ASSUME(code, msg)
#define ASSUME_S(code, msg)
#else
#include <iostream>
#define DEBUG_BREAK() __builtin_trap()

#define ASSUME(block, msg) \
    do { \
        bool __assert_result = ([&]() -> bool block)(); \
        if (!__assert_result) { \
            std::cerr << "Assertion failed: " << (msg) << "\n"; \
            DEBUG_BREAK(); \
            std::abort(); \
        } \
    } while (0)

// short for assume_statement.
#define ASSUME_S(statement, msg) \
    do { \
        if (!(statement)) { \
            std::cerr << "Assertion failed: " << (msg) << "\n"; \
            DEBUG_BREAK(); \
            std::abort(); \
        } \
    } while (0)
#endif


#endif //ASSUME_H
