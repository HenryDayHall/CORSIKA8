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

#include <corsika/media/WeightProvider.hpp>

#include <cassert>
#include <functional>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace corsika {

  NuclearComposition::NuclearComposition(std::vector<Code> const& pComponents,
                                         std::vector<float> const& pFractions)
      : numberFractions_(pFractions)
      , components_(pComponents)
      , avgMassNumber_(std::inner_product(
            pComponents.cbegin(), pComponents.cend(), pFractions.cbegin(), 0.,
            std::plus<double>(), [](auto const compID, auto const fraction) -> double {
              if (is_nucleus(compID)) {
                return get_nucleus_A(compID) * fraction;
              } else {
                return get_mass(compID) / convert_SI_to_HEP(constants::u) * fraction;
              }
            })) {
    assert(pComponents.size() == pFractions.size());
    auto const sumFractions =
        std::accumulate(pFractions.cbegin(), pFractions.cend(), 0.f);

    if (!(0.999f < sumFractions && sumFractions < 1.001f)) {
      throw std::runtime_error("element fractions do not add up to 1");
    }
    this->updateHash();
  }

  template <typename TFunction>
  inline auto NuclearComposition::getWeightedSum(TFunction const& func) const {
    using ResultQuantity = decltype(func(*components_.cbegin()));

    auto const prod = [&](auto const compID, auto const fraction) {
      return func(compID) * fraction;
    };

    if constexpr (phys::units::is_quantity_v<ResultQuantity>) {
      return std::inner_product(
          components_.cbegin(), components_.cend(), numberFractions_.cbegin(),
          ResultQuantity::zero(), // .zero() is defined for quantity types only
          std::plus<ResultQuantity>(), prod);
    } else {
      return std::inner_product(
          components_.cbegin(), components_.cend(), numberFractions_.cbegin(),
          ResultQuantity(0), // in other cases we have to use a bare 0
          std::plus<ResultQuantity>(), prod);
    }
  }

  inline size_t NuclearComposition::getSize() const { return numberFractions_.size(); }

  inline std::vector<float> const& NuclearComposition::getFractions() const {
    return numberFractions_;
  }

  inline std::vector<Code> const& NuclearComposition::getComponents() const {
    return components_;
  }

  inline double const NuclearComposition::getAverageMassNumber() const {
    return avgMassNumber_;
  }

  template <class TRNG>
  inline Code NuclearComposition::sampleTarget(std::vector<CrossSectionType> const& sigma,
                                               TRNG& randomStream) const {

    assert(sigma.size() == numberFractions_.size());

    std::discrete_distribution channelDist(
        WeightProviderIterator<decltype(numberFractions_.begin()),
                               decltype(sigma.begin())>(numberFractions_.begin(),
                                                        sigma.begin()),
        WeightProviderIterator<decltype(numberFractions_.begin()), decltype(sigma.end())>(
            numberFractions_.end(), sigma.end()));

    auto const iChannel = channelDist(randomStream);
    return components_[iChannel];
  }

  // Note: when this class ever modifies its internal data, the hash
  // must be updated, too!
  inline size_t NuclearComposition::getHash() const { return hash_; }

  inline void NuclearComposition::updateHash() {
    std::vector<std::size_t> hashes;
    for (float ifrac : this->getFractions()) hashes.push_back(std::hash<float>{}(ifrac));
    for (Code icode : this->getComponents())
      hashes.push_back(std::hash<int>{}(static_cast<int>(icode)));
    std::size_t h = std::hash<double>{}(this->getAverageMassNumber());
    for (std::size_t ih : hashes) h = h ^ (ih << 1);
    hash_ = h;
  }

} // namespace corsika
