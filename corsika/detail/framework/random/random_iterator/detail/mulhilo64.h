/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2021 Antonio Augusto Alves Junior
 *
 *   This file is part of RandomIterator.
 *
 *   RandomIterator is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   RandomIterator is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with RandomIterator.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/
/*
 * mulhilo64.h
 *
 *  Created on: 12/03/2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

#include <cmath>
#include <cstdint>

#ifdef __powerpc64__
static inline uint64_t  _mulhilo_asm_64(uint64_t ax, uint64_t b, uint64_t *hip){
    uint64_t dx;
    __asm__("\n\t"
        "mulhdu" " %2\n\t"
        : "=a"(ax), "=d"(dx)
        : "r"(b), "0"(ax)
        );
    *hip = dx;
    return ax;
}
#else
static inline uint64_t  _mulhilo_asm_64(uint64_t ax, uint64_t b, uint64_t *hip){
    uint64_t dx;
    __asm__("\n\t"
        "mulq" " %2\n\t"
        : "=a"(ax), "=d"(dx)
        : "r"(b), "0"(ax)
        );
    *hip = dx;
    return ax;
}

#endif /* __powerpc64__ */
