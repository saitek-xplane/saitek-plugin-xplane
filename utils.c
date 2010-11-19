// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#include "utils.h"

unsigned int bcd2dec(unsigned int num, int n) {
    int i;
    unsigned int val = 0;

    for (i = 0; i < n; i++) {
        val += (((num >> (i * 4)) & 0x0F) * (i ? (i * 10):1));
    }

    return val;
}

unsigned int dec2bcd(unsigned int num, int n) {
    int i;
    unsigned int q, r;
    unsigned int val = 0;

    for (i = 0; i < n; i++) {
        q = num / 10;
        r = num % 10;
        val |= (r << (i * 4));
        num = q;
    }

    return val;
}

//// mask_gen returns the bit field width representation of the value x.
//func maskWidth(x uint32) uint32 {
//	return ^(0xFFFFFFFF << x)
//}

//// bitsSet
//func bitsSet(x uint32) uint32 {
//	x = x - ((x >> 1) & 0x55555555)
//	x = (x & 0x33333333) + ((x >> 2) & 0x33333333)

//	return ((x + (x >> 4) & 0xF0F0F0F) * 0x1010101) >> 24
//}

void toggle_bit(unsigned char* c, long pos) {
    *c ^= (0x01 << pos);
}

void to_bytes(unsigned char* c, unsigned long long v) {
    int shift = 32;

    for (int i = 0; i < 5; i++) {
        c[i] = (unsigned char) (v >> shift);
        shift += 8;
    }
}
