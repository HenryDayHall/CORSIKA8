/*----------------------------------------------------------------------------
*
* Copyright (C) 2021 - 2024 Antonio Augusto Alves Junior
*
* This file is part of RandomIterator.

* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS” AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*
* ----------------------------------------------------------------------------*/

/*
 * SplitMix.hpp
 *
 *  Created on: 25/02/2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

namespace random_iterator {

  namespace detail {

    template <typename UIntType>
    inline UIntType splitmix(UIntType&);

    template <>
    inline uint32_t splitmix<uint32_t>(uint32_t& x) {
      uint32_t z = (x += 0x6D2B79F5UL);
      z = (z ^ (z >> 15)) * (z | 1UL);
      z ^= z + (z ^ (z >> 7)) * (z | 61UL);
      return z ^ (z >> 14);
    }

    template <>
    inline uint64_t splitmix<uint64_t>(uint64_t& x) {
      uint64_t z = (x += 0x9e3779b97f4a7c15);
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
      z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
      return z ^ (z >> 31);
    }

  } // namespace detail

} // namespace random_iterator
