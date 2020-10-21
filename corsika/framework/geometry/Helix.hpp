n/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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
   * \class Helix
   *
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

    using VelocityVec = Vector<SpeedType::dimension_type> ;

    Point const r0;
    FrequencyType const omegaC;
    VelocityVec const vPar;
    VelocityVec const vPerp, uPerp;

    LengthType const radius;

  public:
    Helix(Point const& pR0, FrequencyType pOmegaC, VelocityVec const& pvPar,
          VelocityVec const& pvPerp)
        : r0(pR0)
        , omegaC(pOmegaC)
        , vPar(pvPar)
        , vPerp(pvPerp)
        , uPerp(vPerp.cross(vPar.normalized()))
        , radius(pvPar.norm() / abs(pOmegaC)) {}

    inline Point GetPosition(TimeType t) const;

    inline Point PositionFromArclength(LengthType l) const;

    inline LengthType GetRadius() const;

    inline LengthType ArcLength(TimeType t1, TimeType t2) const;

    inline TimeType TimeFromArclength(LengthType l) const;
  };

} // namespace corsika

#include <corsika/detail/framework/geometry/Helix.inl>
