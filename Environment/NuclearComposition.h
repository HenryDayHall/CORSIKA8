
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_NuclearComposition_h
#define _include_NuclearComposition_h

#include <corsika/particles/ParticleProperties.h>
#include <cassert>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace corsika::environment {
  class NuclearComposition {
    std::vector<float> const fNumberFractions; //!< relative fractions of number density
    std::vector<corsika::particles::Code> const
        fComponents; //!< particle codes of consitutents

    double const fAvgMassNumber;

  public:
    NuclearComposition(std::vector<corsika::particles::Code> pComponents,
                       std::vector<float> pFractions)
        : fNumberFractions(pFractions)
        , fComponents(pComponents)
        , fAvgMassNumber(std::inner_product(
              pComponents.cbegin(), pComponents.cend(), pFractions.cbegin(), 0.,
              [](double x, double y) { return x + y; },
              [](auto const& compID, auto const& fraction) {
                return corsika::particles::GetNucleusA(compID) * fraction;
              })) {
      assert(pComponents.size() == pFractions.size());
      auto const sumFractions =
          std::accumulate(pFractions.cbegin(), pFractions.cend(), 0.f);

      if (!(0.999f < sumFractions && sumFractions < 1.001f)) {
        throw std::runtime_error("element fractions do not add up to 1");
      }
    }

    auto size() const { return fNumberFractions.size(); }

    auto const& GetFractions() const { return fNumberFractions; }
    auto const& GetComponents() const { return fComponents; }
    auto const GetAverageMassNumber() const { return fAvgMassNumber; }
  };

} // namespace corsika::environment

#endif
