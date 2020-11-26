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

  /** Describes the composition of matter
   *  Allowes and handles the creation of custom matter compositions
   **/
  class NuclearComposition {
  private:
    /// TODO: replace with
    /// https://www.boost.org/doc/libs/1_74_0/libs/iterator/doc/zip_iterator.html or
    /// ranges zip
    /** Double Iterator
     * Iterator that allowes the iteration of two individual lists at the same time. The
     *user needs to take care that booth lists have the same length.
     *  @tparam AConstIterator Iterator Type of the first list
     *  @tparam BConstIterator Iterator Type of the second list
     **/
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
    /** Constructor
     *  The constructore takes a list of elements and a list which describe the relative
     *  amount. Booth lists need to have the same length and the sum all of fractions
     *  should be 1. Otherwise an exception is thrown
     *  @param pComponents List of particle types
     *  @param pFractions List of fractions how much each particle contributes. The sum
     *needs to add up to 1
     **/
    NuclearComposition(std::vector<corsika::Code> pComponents,
                       std::vector<float> pFractions);

    /** Sum all all relative composition weighted by func(element)
     *  This function sums all relative compositions given during this classes
     *construction. Each entry is weighted by the user defined function func given to this
     *function.
     *  @tparam TFunction Type of functions for the weights. The type should be
     *corsika::Code -> float
     *  @param func Functions for reweighting specific elements
     *  @retval returns the weighted sum with the type defined by the return type of func
     **/
    template <typename TFunction>
    auto weightedSum(TFunction func) const;

    /** Number of elements in the composition array
     *  @retval returns the number of elements in the composition array
     **/
    auto size() const;

    /// Returns a const reference to the fraction
    std::vector<float> const& getFractions() const;
    /// Returns a const reference to the fraction
    std::vector<corsika::Code> const& getComponents() const;
    auto const getAverageMassNumber() const;

    template <class TRNG>
    Code sampleTarget(std::vector<units::si::CrossSectionType> const& sigma,
                      TRNG& randomStream) const;

    // Note: when this class ever modifies its internal data, the hash
    // must be updated, too!
    size_t hash() const;

  private:
    void updateHash();

    std::vector<float> const numberFractions_; //!< relative fractions of number density
    std::vector<corsika::Code> const components_; //!< particle codes of consitutents

    double const avgMassNumber_;

    std::size_t hash_;
  };

} // namespace corsika

#include <corsika/detail/media/NuclearComposition.inl>
