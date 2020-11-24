/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
  class NuclearComposition {
    std::vector<float> const numberFractions_; //!< relative fractions of number density
    std::vector<corsika::Code> const
        components_; //!< particle codes of consitutents

    double const avgMassNumber_;

    std::size_t hash_;

    template <class AConstIterator, class BConstIterator>
    class WeightProviderIterator {
      AConstIterator aIter_;
      BConstIterator bIter_;

    public:
      using value_type = double;
      using iterator_category = std::input_iterator_tag;
      using pointer = value_type*;
      using reference = value_type&;
      using difference_type = ptrdiff_t;

      WeightProviderIterator(AConstIterator a, BConstIterator b);

      value_type operator*() const;

      WeightProviderIterator& operator++();

      auto operator==(WeightProviderIterator other);

      auto operator!=(WeightProviderIterator other);
    };

  public:
    NuclearComposition(std::vector<corsika::Code> pComponents,
                       std::vector<float> pFractions);

    template <typename TFunction>
    auto WeightedSum(TFunction func) const ;

    auto size() const;

    auto const& GetFractions() const;
    auto const& GetComponents() const ;
    auto const GetAverageMassNumber() const;

    template <class TRNG>
    Code SampleTarget(
        std::vector<units::si::CrossSectionType> const& sigma,
        TRNG& randomStream) const;

    // Note: when this class ever modifies its internal data, the hash
    // must be updated, too!
    size_t hash() const;

  private:
    void updateHash();
  };

} // namespace corsika

#include <corsika/detail/media/NuclearComposition.inl>
