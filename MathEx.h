/**
 * Copyright (c) Jorma Rebane - 2013
 */
#pragma once
#ifndef MATH_EX_H
#define MATH_EX_H

/**
 * @return Upper nearest power of two value
 */
inline size_t upper_pow2(size_t v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}


#endif
