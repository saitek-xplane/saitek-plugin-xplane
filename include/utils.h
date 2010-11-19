// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

    extern unsigned int bcd2dec(unsigned int num, int n);
    extern unsigned int dec2bcd(unsigned int num, int n);
    extern void toggle_bit(unsigned char* c, long pos);
    extern void to_bytes(unsigned char* c, unsigned long long v);

#ifdef __cplusplus
}
#endif

#endif
