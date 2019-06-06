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
#include <corsika/units/PhysicalUnits.h>

#include <cassert>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace corsika::environment {
  class NuclearComposition {
    std::vector<float> const fNumberFractions; //!< relative fractions of number density
    std::vector<corsika::particles::Code> const
        fComponents; //!< particle codes of consitutents

    double const fAvgMassNumber;

    template <class AConstIterator, class BConstIterator>
    class WeightProviderIterator {
      AConstIterator fAIter;
      BConstIterator fBIter;

    public:
      using value_type = double;
      using iterator_category = std::input_iterator_tag;
      using pointer = value_type*;
      using reference = value_type&;
      using difference_type = ptrdiff_t;

      WeightProviderIterator(AConstIterator a, BConstIterator b)
          : fAIter(a)
          , fBIter(b) {}

      value_type operator*() const { return ((*fAIter) * (*fBIter)).magnitude(); }

      WeightProviderIterator& operator++() { // prefix ++
        ++fAIter;
        ++fBIter;
        return *this;
      }

      auto operator==(WeightProviderIterator other) { return fAIter == other.fAIter; }

      auto operator!=(WeightProviderIterator other) { return !(*this == other); }
    };

  public:
    NuclearComposition(std::vector<corsika::particles::Code> pComponents,
                       std::vector<float> pFractions)
        : fNumberFractions(pFractions)
        , fComponents(pComponents)
        , fAvgMassNumber(std::inner_product(
              pComponents.cbegin(), pComponents.cend(), pFractions.cbegin(), 0.,
              std::plus<double>(), [](auto const compID, auto const fraction) -> double {
                if (particles::IsNucleus(compID)) {
                  return particles::GetNucleusA(compID) * fraction;
                } else {
                  return particles::GetMass(compID) /
                         units::si::ConvertSIToHEP(units::constants::u) * fraction;
                }
              })) {
      assert(pComponents.size() == pFractions.size());
      auto const sumFractions =
          std::accumulate(pFractions.cbegin(), pFractions.cend(), 0.f);

      if (!(0.999f < sumFractions && sumFractions < 1.001f)) {
        throw std::runtime_error("element fractions do not add up to 1");
      }
    }

    template <typename TFunction>
    auto WeightedSum(TFunction func) const {
      using ResultQuantity = decltype(func(*fComponents.cbegin()));

      auto const sum = [](auto x, auto y) { return x + y; };
      auto const prod = [&](auto const compID, auto const fraction) {
        return func(compID) * fraction;
      };

      if constexpr (phys::units::is_quantity_v<ResultQuantity>) {
        return std::inner_product(
            fComponents.cbegin(), fComponents.cend(), fNumberFractions.cbegin(),
            ResultQuantity::zero(), // .zero() is defined for quantity types only
            sum, prod);
      } else {
        return std::inner_product(
            fComponents.cbegin(), fComponents.cend(), fNumberFractions.cbegin(),
            ResultQuantity(0), // in other cases we have to use a bare 0
            sum, prod);
      }
    }

    auto size() const { return fNumberFractions.size(); }

    auto const& GetFractions() const { return fNumberFractions; }
    auto const& GetComponents() const { return fComponents; }
    auto const GetAverageMassNumber() const { return fAvgMassNumber; }

    template <class TRNG>
    corsika::particles::Code SampleTarget(
        std::vector<corsika::units::si::CrossSectionType> const& sigma,
        TRNG& randomStream) const {
      using namespace corsika::units::si;

      assert(sigma.size() == fNumberFractions.size());

      std::discrete_distribution channelDist(
          WeightProviderIterator<decltype(fNumberFractions.begin()),
                                 decltype(sigma.begin())>(fNumberFractions.begin(),
                                                          sigma.begin()),
          WeightProviderIterator<decltype(fNumberFractions.begin()),
                                 decltype(sigma.end())>(fNumberFractions.end(),
                                                        sigma.end()));

      auto const iChannel = channelDist(randomStream);
      return fComponents[iChannel];
    }
  };

} // namespace corsika::environment

#endif
