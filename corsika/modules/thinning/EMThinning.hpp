/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <random>

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>

namespace corsika {

  //! This process implements thinning for EM splitting processes (1 -> 2)

  class EMThinning : public SecondariesProcess<EMThinning> {
  public:
    /**
     * Construct a new EMThinning process.
     *
     * @param threshold: thinning applied below this energy
     * @param maxWeight: maximum allowed weight
     */
    EMThinning(HEPEnergyType threshold, double maxWeight);

    /**
     * Apply thinning to secondaries. Only EM primaries with two EM secondaries are
     * considered.
     *
     * @tparam TStackView
     */
    template <typename TStackView>
    void doSecondaries(TStackView&);

  private:
    default_prng_type& rng_ = RNGManager<>::getInstance().getRandomStream("thinning");
    std::uniform_real_distribution<double> uniform_{};
    HEPEnergyType const threshold_;
    double const maxWeight_;
  };
} // namespace corsika

#include <corsika/detail/modules/thinning/EMThinning.inl>
