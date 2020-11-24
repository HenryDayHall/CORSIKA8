/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <cassert>
#include <functional>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace corsika {

  namespace nuc_comp::detail {

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

  } // namespace nuc_comp::detail

  class NuclearComposition {
    std::vector<float> const fNumberFractions; //!< relative fractions of number density
    std::vector<corsika::Code> const fComponents; //!< particle codes of consitutents

    double const fAvgMassNumber;

    /*
     * FIXME: smelling here... nested class why? ...done
     * FIXME: promote it to namespace detail ... done
     */

  public:
    NuclearComposition(std::vector<corsika::Code> pComponents,
                       std::vector<float> pFractions)
        : fNumberFractions(pFractions)
        , fComponents(pComponents)
        , fAvgMassNumber(std::inner_product(
              pComponents.cbegin(), pComponents.cend(), pFractions.cbegin(), 0.,
              std::plus<double>(), [](auto const compID, auto const fraction) -> double {
                if (IsNucleus(compID)) {
                  return GetNucleusA(compID) * fraction;
                } else {
                  return GetMass(compID) / ConvertSIToHEP(constants::u) * fraction;
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

      auto const prod = [&](auto const compID, auto const fraction) {
        return func(compID) * fraction;
      };

      if constexpr (phys::units::is_quantity_v<ResultQuantity>) {
        return std::inner_product(
            fComponents.cbegin(), fComponents.cend(), fNumberFractions.cbegin(),
            ResultQuantity::zero(), // .zero() is defined for quantity types only
            std::plus<ResultQuantity>(), prod);
      } else {
        return std::inner_product(
            fComponents.cbegin(), fComponents.cend(), fNumberFractions.cbegin(),
            ResultQuantity(0), // in other cases we have to use a bare 0
            std::plus<ResultQuantity>(), prod);
      }
    }

    auto size() const { return fNumberFractions.size(); }

    auto const& GetFractions() const { return fNumberFractions; }
    auto const& GetComponents() const { return fComponents; }
    auto const GetAverageMassNumber() const { return fAvgMassNumber; }

    template <class TRNG>
    corsika::Code SampleTarget(std::vector<CrossSectionType> const& sigma,
                               TRNG& randomStream) const {

      assert(sigma.size() == fNumberFractions.size());

      std::discrete_distribution channelDist(
          nuc_comp::detail::WeightProviderIterator<decltype(fNumberFractions.begin()),
                                                   decltype(sigma.begin())>(
              fNumberFractions.begin(), sigma.begin()),
          nuc_comp::detail::WeightProviderIterator<decltype(fNumberFractions.begin()),
                                                   decltype(sigma.end())>(
              fNumberFractions.end(), sigma.end()));

      auto const iChannel = channelDist(randomStream);
      return fComponents[iChannel];
    }
  };

} // namespace corsika
