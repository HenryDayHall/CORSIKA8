/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one
                          // cpp file

#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/particles/ParticleProperties.h>
#include <catch2/catch.hpp>
#include <random>
#include <vector>

using namespace corsika::geometry;
using namespace corsika::environment;
using namespace corsika::units::si;

TEST_CASE("HomogeneousMedium") {
  NuclearComposition const protonComposition(
      std::vector<corsika::particles::Code>{corsika::particles::Code::Proton},
      std::vector<float>{1.f});
  HomogeneousMedium<IMediumModel> const medium(19.2_g / cube(1_cm), protonComposition);
}

TEST_CASE("NuclearComposition") {
  NuclearComposition const composition(
      std::vector<corsika::particles::Code>{corsika::particles::Code::Proton,
                                            corsika::particles::Code::Neutron},
      std::vector<float>{2.f / 3.f, 1.f / 3.f});
  SECTION("SampleTarget") {
    std::vector<CrossSectionType> crossSections{50_mbarn, 100_mbarn};

    std::mt19937 rng;

    int proton{0}, neutron{0};

    for (int i = 0; i < 1'000'000; ++i) {
      corsika::particles::Code p = composition.SampleTarget(crossSections, rng);
      switch (p) {
        case corsika::particles::Code::Proton:
          proton++;
          break;
        case corsika::particles::Code::Neutron:
          neutron++;
          break;
        default:
          throw std::runtime_error("");
      }
    }

    REQUIRE(static_cast<double>(proton) / neutron == Approx(1).epsilon(1e-2));
  }
}
