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
 * EngineTraits.hpp
 *
 *  Created on: 23 de fev. de 2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

#include <stdint.h>
#include "Random123/array.h"
#include "Random123/philox.h"
#include "Random123/threefry.h"
#include "Random123/ars.h"
#include "Random123/ReinterpretCtr.hpp"

namespace random_iterator {

  namespace detail {

    template <typename Engine>
    struct random_traits;
    /*
     * random_traits<T>::state_type { counter, state}
     * random_traits<T>::advance_type;
     * random_traits<T>::init_type;
     * random_traits<T>::result_type;
     */

    // philox
    template <>
    struct random_traits<random_iterator_r123::Philox2x64> {
      typedef typename random_iterator_r123::Philox2x64::ctr_type state_type;
      typedef typename random_iterator_r123::Philox2x64::key_type seed_type;
      typedef typename random_iterator_r123::Philox2x64::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 2 };
    };

    template <>
    struct random_traits<random_iterator_r123::Philox4x64> {
      typedef typename random_iterator_r123::Philox4x64::ctr_type state_type;
      typedef typename random_iterator_r123::Philox4x64::key_type seed_type;
      typedef typename random_iterator_r123::Philox4x64::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 4 };
    };
    //
    template <>
    struct random_traits<random_iterator_r123::Threefry2x64> {
      typedef typename random_iterator_r123::Threefry2x64::ctr_type state_type;
      typedef typename random_iterator_r123::Threefry2x64::key_type seed_type;
      typedef typename random_iterator_r123::Threefry2x64::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 2 };
    };

    //
    template <>
    struct random_traits<random_iterator_r123::Threefry4x64> {
      typedef typename random_iterator_r123::Threefry4x64::ctr_type state_type;
      typedef typename random_iterator_r123::Threefry4x64::key_type seed_type;
      typedef typename random_iterator_r123::Threefry4x64::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 4 };
    };

#if RANDOM_ITERATOR_R123_USE_AES_NI
    template <>
    struct random_traits<random_iterator_r123::ARS4x32> {
      typedef typename random_iterator_r123::ARS4x32::ctr_type state_type;
      typedef typename random_iterator_r123::ARS4x32::key_type seed_type;
      typedef typename random_iterator_r123::ARS4x32::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 4 };
    };

    template <>
    struct random_traits<random_iterator_r123::ARS2x64> {
      typedef typename random_iterator_r123::ARS2x64::ctr_type state_type;
      typedef typename random_iterator_r123::ARS2x64::key_type seed_type;
      typedef typename random_iterator_r123::ARS2x64::ukey_type init_type;
      typedef uint64_t advance_type;
      typedef state_type::value_type result_type;

      enum { arity = 2 };
    };

#endif

  } // namespace detail

} // namespace random_iterator
