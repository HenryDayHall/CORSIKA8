/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <cmath>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>

namespace corsika {
  /*!
   * A Helix is defined by the cyclotron frequency \f$ \omega_c \f$, the initial
   * Point r0 and
   * the velocity vectors \f$ \vec{v}_{\parallel} \f$ and \f$ \vec{v}_{\perp} \f$
   * denoting the projections of the initial velocity \f$ \vec{v}_0 \f$ parallel
   * and perpendicular to the axis \f$ \vec{B} \f$, respectively, i.e.
   * \f{align*}{
        \vec{v}_{\parallel} &= \frac{\vec{v}_0 \cdot \vec{B}}{\vec{B}^2} \vec{B} \\
        \vec{v}_{\perp} &= \vec{v}_0 - \vec{v}_{\parallel}
     \f}
   */

  class Helix {

    using VelocityVec = Vector<units::si::SpeedType::dimension_type>;

    Point const r0;
    units::si::FrequencyType const omegaC;
    VelocityVec const vPar;
    VelocityVec const vPerp, uPerp;

    corsika::units::si::LengthType const radius;

  public:
    Helix(Point const& pR0, units::si::FrequencyType pOmegaC, VelocityVec const& pvPar,
          VelocityVec const& pvPerp)
        : r0(pR0)
        , omegaC(pOmegaC)
        , vPar(pvPar)
        , vPerp(pvPerp)
        , uPerp(vPerp.cross(vPar.normalized()))
        , radius(pvPar.norm() / abs(pOmegaC)) {}

    inline Point GetPosition(units::si::TimeType t) const;

    inline Point PositionFromArclength(units::si::LengthType l) const;

    inline units::si::LengthType GetRadius() const;

    inline units::si::LengthType ArcLength(units::si::TimeType t1,
                                           units::si::TimeType t2) const;

    inline units::si::TimeType TimeFromArclength(units::si::LengthType l) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Helix.inl>
