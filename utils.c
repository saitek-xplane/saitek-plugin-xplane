// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include <stdio.h>
#include <stdint.h>
#include <math.h>

//#include "overloaded.h"
#include "defs.h"
#include "utils.h"

uint32_t bcd2dec(uint32_t num, int32_t n) {
    uint32_t val = 0;

    for (int i = 0; i < n; i++) {
        val += (uint32_t) (((num >> (i * 4)) & 0x0F) * pow(10, i));
    }

    return val;
}

uint32_t dec2bcd(uint32_t num, int32_t n) {
    uint32_t q, r;
    uint32_t val = 0;

    for (int i = 0; i < n; i++) {
        q = num / 10;
        r = num % 10;
        val |= (r << (i * 4));
        num = q;
    }

    return val;
}

//void toggle_bit(uint8_t* c, int32_t pos) {
//
//    *c ^= (0x01 << pos);
//}

void to_bytes(uint8_t* c, uint64_t v) {
    int shift = 32;

    for (int i = 0; i < 5; i++) {
        c[i] = (uint8_t) (v >> shift);
        shift += 8;
    }
}
