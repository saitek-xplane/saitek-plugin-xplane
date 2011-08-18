// Copyright (c) 2010 Joseph D Poirier
// Distributable under the terms of The New BSD License
// that can be found in the LICENSE file.

#ifndef UTILS_H
#define UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

    extern uint32_t bcd2dec(uint32_t num, int32_t n);
    extern uint32_t dec2bcd(uint32_t num, int32_t n);
    extern void toggle_bit(uint8_t* c, int32_t pos);
    extern void to_bytes(uint8_t* c, uint64_t v);

#ifdef __cplusplus
}
#endif

#endif /* UTILS_H */
