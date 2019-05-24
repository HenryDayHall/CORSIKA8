/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_ExponentialDistribution_h
#define _include_ExponentialDistribution_h

#include <corsika/units/PhysicalUnits.h>
#include <random>

namespace corsika::random {

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

} // namespace corsika::random

#endif
