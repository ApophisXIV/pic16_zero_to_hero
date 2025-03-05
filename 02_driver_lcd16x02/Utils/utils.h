#ifndef UTILS_H
#define UTILS_H

#define SET_BIT(x, y)      (x |= (1 << y))
#define CLEAR_BIT(x, y)    (x &= ~(1 << y))
#define TOGGLE_BIT(x, y)   (x ^= (1 << y))
#define IS_BIT_SET(x, y)   (x & (1 << y))
#define IS_BIT_CLEAR(x, y) (!(x & (1 << y)))

#define IS_SINGLE_BIT_SET(x) (x && !(x & (x - 1)))

#endif    // UTILS_H