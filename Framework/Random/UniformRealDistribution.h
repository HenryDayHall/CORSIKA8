/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_UniformRealDistribution_h
#define _include_UniformRealDistribution_h

#include <corsika/units/PhysicalUnits.h>
#include <random>

namespace corsika::random {

  template <class TQuantity>
  class UniformRealDistribution {
    using RealType = typename TQuantity::value_type;
    std::uniform_real_distribution<RealType> dist{RealType(0.), RealType(1.)};

    TQuantity const a, b;

  public:
    UniformRealDistribution(TQuantity b)
        : a{TQuantity(phys::units::detail::magnitude_tag, 0)}
        , b(b) {}
    UniformRealDistribution(TQuantity a, TQuantity b)
        : a(a)
        , b(b) {}

    template <class Generator>
    TQuantity operator()(Generator& g) {
      return a + dist(g) * (b - a);
    }
  };

} // namespace corsika::random

#endif
