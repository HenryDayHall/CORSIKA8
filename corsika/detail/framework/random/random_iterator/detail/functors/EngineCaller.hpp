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
 * EngineCaller.hpp
 *
 *  Created on: 23/02/2021
 *      Author: Antonio Augusto Alves Junior
 */

#pragma once

#include <stdint.h>

namespace random_iterator {

  namespace detail {

    template <typename DistibutionType, typename EngineType>
    struct EngineCaller {
      typedef EngineType engine_type;
      typedef typename engine_type::state_type state_type;
      typedef typename engine_type::seed_type seed_type;
      typedef typename engine_type::advance_type advance_type;
      typedef typename engine_type::init_type init_type;

      typedef DistibutionType distribution_type;
      typedef typename distribution_type::result_type result_type;

      EngineCaller() = delete;

      EngineCaller(distribution_type const& dist, seed_type seed, uint32_t stream)
          : distribution_(dist)
          , seed_(seed)
          , stream_(stream) {}

      EngineCaller(EngineCaller<DistibutionType, EngineType> const& other)
          : distribution_(other.distribution_)
          , seed_(other.seed_)
          , stream_(other.stream_) {}

      EngineCaller<DistibutionType, EngineType>& operator=(
          EngineCaller<DistibutionType, EngineType> const& other) {

        if (this == &other) return *this;

        distribution_ = other.distribution_;
        seed_ = other.seed_;
        stream_ = other.stream_;

        return *this;
      }

      inline result_type operator()(advance_type n) const {

        EngineType eng(seed_, stream_);
        eng.discard(n);

        return static_cast<distribution_type>(distribution_)(eng);
      }

      distribution_type distribution_;
      seed_type seed_;
      uint32_t stream_;
    };

  } // namespace detail

} // namespace random_iterator
