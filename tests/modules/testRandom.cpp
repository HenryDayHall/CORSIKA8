/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>
#ifdef WITH_FLUKA
#include <corsika/modules/FLUKA.hpp>
#endif
#include <corsika/modules/Epos.hpp>
#include <corsika/modules/CONEX.hpp>
#include <corsika/modules/Sibyll.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/modules/QGSJetII.hpp>
#include <corsika/modules/Sophia.hpp>
#include <corsika/modules/UrQMD.hpp>

static std::size_t first{};
void dummy_rng_func(double* dest, std::size_t N) {
  double constexpr factor = 1.e-6;
  for (std::size_t i = 0; i < N; ++i) { dest[i] = (first + i) * factor; }
}

TEST_CASE("set_rng") {
  first = 1;
  int dummy{};
  set_sophia_rng_function(dummy_rng_func);
  // call Sophia RNG, which uses injected function
  CHECK(rndm_(dummy) == 1 * 1.e-6);
  CHECK(rndm_(dummy) == 2 * 1.e-6);

  first = 20;
  set_epos_rng_function(dummy_rng_func);
  CHECK(epos::rangen_() == static_cast<float>(20 * 1.e-6));
  CHECK(epos::drangen_() == 21 * 1.e-6);

  first = 40;
  set_sibyll_rng_function(dummy_rng_func);
  CHECK(s_rndm_(dummy) == 40 * 1.e-6);
  CHECK(s_rndm_(dummy) == 41 * 1.e-6);

  first = 60;
  set_qgsjetII_rng_function(dummy_rng_func);
  CHECK(qgran_(dummy) == 60 * 1.e-6);
  CHECK(qgran_(dummy) == 61 * 1.e-6);

  first = 80;
  set_urqmd_rng_function(dummy_rng_func);
  CHECK(urqmd::ranf_(dummy) == 80 * 1.e-6);
  CHECK(urqmd::ranf_(dummy) == 81 * 1.e-6);

  // Sophia is still untouched
  CHECK(rndm_(dummy) == 3 * 1.e-6);

#ifdef WITH_FLUKA
  first = 100;
  set_fluka_rng_function(dummy_rng_func);
  CHECK(fluka::flrndm_(dummy) == 100 * 1.e-6);
  CHECK(fluka::flrndm_(dummy) == 101 * 1.e-6);
#endif
}
