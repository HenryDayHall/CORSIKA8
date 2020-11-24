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

  template <class AConstIterator, class BConstIterator>
  NuclearComposition::WeightProviderIterator<
      AConstIterator, BConstIterator>::WeightProviderIterator(AConstIterator a,
                                                              BConstIterator b)
      : aIter_(a)
      , bIter_(b) {}

  template <class AConstIterator, class BConstIterator>
  typename NuclearComposition::WeightProviderIterator<AConstIterator,
                                                      BConstIterator>::value_type
  NuclearComposition::WeightProviderIterator<AConstIterator, BConstIterator>::operator*()
      const {
    return ((*aIter_) * (*bIter_)).magnitude();
  }

  template <class AConstIterator, class BConstIterator>
  NuclearComposition::WeightProviderIterator<AConstIterator, BConstIterator>&
  NuclearComposition::WeightProviderIterator<AConstIterator,
                                             BConstIterator>::operator++() { // prefix ++
    ++aIter_;
    ++bIter_;
    return *this;
  }

  template <class AConstIterator, class BConstIterator>
  auto
  NuclearComposition::WeightProviderIterator<AConstIterator, BConstIterator>::operator==(
      WeightProviderIterator other) {
    return aIter_ == other.aIter_;
  }

  template <class AConstIterator, class BConstIterator>
  auto
  NuclearComposition::WeightProviderIterator<AConstIterator, BConstIterator>::operator!=(
      WeightProviderIterator other) {
    return !(*this == other);
  }

  NuclearComposition::NuclearComposition(std::vector<corsika::Code> pComponents,
                                         std::vector<float> pFractions)
      : numberFractions_(pFractions)
      , components_(pComponents)
      , avgMassNumber_(std::inner_product(
            pComponents.cbegin(), pComponents.cend(), pFractions.cbegin(), 0.,
            std::plus<double>(), [](auto const compID, auto const fraction) -> double {
              if (IsNucleus(compID)) {
                return GetNucleusA(compID) * fraction;
              } else {
                return GetMass(compID) / units::si::ConvertSIToHEP(constants::u) * fraction;
              }
            })) {
    assert(pComponents.size() == pFractions.size());
    auto const sumFractions =
        std::accumulate(pFractions.cbegin(), pFractions.cend(), 0.f);

    if (!(0.999f < sumFractions && sumFractions < 1.001f)) {
      throw std::runtime_error("element fractions do not add up to 1");
    }
    updateHash();
  }

  template <typename TFunction>
  auto NuclearComposition::WeightedSum(TFunction func) const {
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

  auto NuclearComposition::size() const { return numberFractions_.size(); }

  auto const& NuclearComposition::GetFractions() const { return numberFractions_; }
  auto const& NuclearComposition::GetComponents() const { return components_; }
  auto const NuclearComposition::GetAverageMassNumber() const { return avgMassNumber_; }

  template <class TRNG>
  corsika::Code NuclearComposition::SampleTarget(
      std::vector<units::si::CrossSectionType> const& sigma,
      TRNG& randomStream) const {
    using namespace units::si;

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
  size_t NuclearComposition::hash() const { return hash_; }

  void NuclearComposition::updateHash() {
    std::vector<std::size_t> hashes;
    for (float ifrac : GetFractions()) hashes.push_back(std::hash<float>{}(ifrac));
    for (corsika::Code icode : GetComponents())
      hashes.push_back(std::hash<int>{}(static_cast<int>(icode)));
    std::size_t h = std::hash<double>{}(GetAverageMassNumber());
    for (std::size_t ih : hashes) h = h ^ (ih << 1);
    hash_ = h;
  }

} // namespace corsika
