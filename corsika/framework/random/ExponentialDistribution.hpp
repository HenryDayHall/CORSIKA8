n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <random>

namespace corsika {
  // FIXME: This whole facility needs to re-designed.
  // It is not parallel friendly neither polymorphic
  // and the streaming management is prone to produce
  // huge correlation between the streams

  template <class TQuantity>
  class ExponentialDistribution {
    using RealType = typename TQuantity::value_type;
    std::exponential_distribution<RealType> dist{1.};

    TQuantity const fBeta;

  public:
    ExponentialDistribution(TQuantity beta)
        : fBeta(beta) {}

    template <class Generator>
    TQuantity operator()(Generator& g) {
      return fBeta * dist(g);
    }
  };

} // namespace corsika
