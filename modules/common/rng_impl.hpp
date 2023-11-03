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

alignas(64) static std::array<double, 16> random_buffer{}; // aligned to fill two cache lines
static std::function<void(double*, size_t)> rng_ptr = nullptr;
static double const* next_rand = std::next(random_buffer.data(), random_buffer.size());

#define IMPLEMENT_RNG(NAME) \
namespace NAME { \
void set_rng_function(std::function<void(double*, size_t)> rng_function) { \
  rng_ptr = rng_function; \
} \
} \
extern "C" void set_##NAME##_rng_function(void (*rng_function)(double*, size_t)) { \
  NAME::set_rng_function(std::function{rng_function}); \
}

static double draw_std_rnd() {
  if (next_rand == std::next(random_buffer.data(), random_buffer.size())) {
    // no more unused values in buffer, refill via injected RNG function
    rng_ptr(random_buffer.data(), random_buffer.size());
    next_rand = random_buffer.data();
  }

  return *next_rand++;
}
