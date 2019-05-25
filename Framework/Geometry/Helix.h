/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_HELIX_H_
#define _include_HELIX_H_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

namespace corsika::geometry {
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

    using VelocityVec = Vector<corsika::units::si::SpeedType::dimension_type>;

    Point const r0;
    corsika::units::si::FrequencyType const omegaC;
    VelocityVec const vPar;
    VelocityVec const vPerp, uPerp;

    corsika::units::si::LengthType const radius;

  public:
    Helix(Point const& pR0, corsika::units::si::FrequencyType pOmegaC,
          VelocityVec const& pvPar, VelocityVec const& pvPerp)
        : r0(pR0)
        , omegaC(pOmegaC)
        , vPar(pvPar)
        , vPerp(pvPerp)
        , uPerp(vPerp.cross(vPar.normalized()))
        , radius(pvPar.norm() / abs(pOmegaC)) {}

    Point GetPosition(corsika::units::si::TimeType t) const {
      return r0 + vPar * t +
             (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }

    Point PositionFromArclength(corsika::units::si::LengthType l) const {
      return GetPosition(TimeFromArclength(l));
    }

    auto GetRadius() const { return radius; }

    corsika::units::si::LengthType ArcLength(corsika::units::si::TimeType t1,
                                             corsika::units::si::TimeType t2) const {
      return (vPar + vPerp).norm() * (t2 - t1);
    }

    corsika::units::si::TimeType TimeFromArclength(
        corsika::units::si::LengthType l) const {
      return l / (vPar + vPerp).norm();
    }
  };

} // namespace corsika::geometry

#endif
