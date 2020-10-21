/*
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

  // FIXME: This while facility needs to re-designed.
  // It is not parallel friendly neither polymorphic
  // and the streaming management is prone to produce
  // huge correlation between the streams

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

} // namespace corsika
