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
 * Macros.h
 *
 *  Created on: 04/03/2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

#if defined(__has_builtin)
    //__has_builtin(__builtin_expect)
	#if __has_builtin(__builtin_expect)
		 #define   _LIKELY_(x) __builtin_expect(x, 1)
		 #define _UNLIKELY_(x) __builtin_expect(x, 0)
	#else
		 #define   _LIKELY_(x) x
		 #define _UNLIKELY_(x) x
	#endif //__has_builtin(__builtin_expect)
#else
	#define   _LIKELY_(x) x
	#define _UNLIKELY_(x) x
#endif //defined(__has_builtin)


//detect endianess
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) ||                               \
    defined(__ARMEB__) ||                                    \
    defined(__THUMBEB__) ||                                  \
    defined(__AARCH64EB__) ||                                \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
#ifndef __BIG_ENDIAN__
#define __BIG_ENDIAN__
#endif
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) ||                                 \
    defined(__ARMEL__) ||                                         \
    defined(__THUMBEL__) ||                                       \
    defined(__AARCH64EL__) ||                                     \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || \
    defined(_WIN32) || defined(__i386__) || defined(__x86_64__) || \
    defined(_X86_) || defined(_IA64_)
#ifndef __LITTLE_ENDIAN__
#define __LITTLE_ENDIAN__
#endif
#else
#error "I don't know what architecture this is!"
#endif
