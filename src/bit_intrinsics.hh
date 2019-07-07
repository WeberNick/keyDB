#pragma once

//#include <inttypes.h>
#include <cstdint>

#ifdef __x86_64
#include "immintrin.h"
#endif


template <class Tuint>
inline uint32_t idx_lowest_bit_set(const Tuint x);

template <>
inline uint32_t idx_lowest_bit_set<uint16_t>(const uint16_t x) {
#ifdef __ICC
    return (_bit_scan_forward(x));
#elif defined __GNUG__
    return static_cast<uint32_t>(__builtin_ctz(x));
#endif
}

template <>
inline uint32_t idx_lowest_bit_set<uint32_t>(const uint32_t x) {
#ifdef __ICC
    return (_bit_scan_forward(x));
#elif defined __GNUG__
    return static_cast<uint32_t>(__builtin_ctz(x));
#endif
}

template <>
inline uint32_t idx_lowest_bit_set<uint64_t>(const uint64_t x) {
#ifdef __ICC
    return (__builtin_ctzll(x)); // gibt kein _bit_scan_forward64
#elif defined __GNUG__
    return static_cast<uint32_t>(__builtin_ctzll(x));
#endif
}
