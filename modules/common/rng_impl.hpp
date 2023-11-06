/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <array>
#include <functional>
#include <iterator>
#include <iostream>

// buffer of random numbers to be filled all at once and then used one at a time
// sized and aligned to fill two cache lines (assuming 64 byte size)
alignas(64) static std::array<double, 16> random_buffer{};

// The callable to be injected. Supposed to fill an array of doubles
// with standard random numbers.
static std::function<void(double*, size_t)> rng_ptr = nullptr;

// pointer to next number in buffer to be returned
static double const* next_rand = std::next(random_buffer.data(), random_buffer.size());

/** use this macro to implement a function set_rng_function() in namespace NAME together
 * with a C function set_NAME_rng_function(). While set_NAME_rng_function() expects a
 * function pointer as argument, NAME::set_rng_function() expects a std::function object
 */
#define IMPLEMENT_RNG(NAME)                                                          \
  namespace NAME {                                                                   \
    void set_rng_function(std::function<void(double*, size_t)> rng_function) {       \
      std::cout << __PRETTY_FUNCTION__ << " called" << std::endl;                    \
      rng_ptr = rng_function;                                                        \
    }                                                                                \
  }                                                                                  \
  extern "C" void set_##NAME##_rng_function(void (*rng_function)(double*, size_t)) { \
    NAME::set_rng_function(std::function{rng_function});                             \
  }

/** Draw one standard random number from the buffer. If exhausted, it will be refilled
 * with new numbers using the injected function.
 */
static double draw_std_rnd() {
  static bool printed = false;
  if (!printed) {
    std::cout << __PRETTY_FUNCTION__ << " called for the first time; using buffer at "
              << random_buffer.data() << std::endl;
    printed = true;
  }

  if (next_rand == std::next(random_buffer.data(), random_buffer.size())) {
    // no more unused values in buffer, refill via injected RNG function
    rng_ptr(random_buffer.data(), random_buffer.size());
    next_rand = random_buffer.data();
  }

  return *next_rand++;
}
