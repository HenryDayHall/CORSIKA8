/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_BaseExponential_h_
#define _include_Environment_BaseExponential_h_

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

#include <cassert>
#include <limits>

namespace corsika::environment {

  /**
   * This class provides the grammage/length conversion functionality for
   * (locally) flat exponential atmospheres.
   */
  
  template <class TDerived>
  class BaseExponential {
  protected:
    units::si::MassDensityType const fRho0;
    units::si::LengthType const fLambda;
    units::si::InverseLengthType const fInvLambda;
    geometry::Point const fP0;

    auto const& GetImplementation() const { return *static_cast<TDerived const*>(this); }

    // clang-format off
    /**
     * For a (normalized) axis \f$ \vec{a} \f$, the grammage along a non-orthogonal line with (normalized)
     * direction \f$ \vec{v} \f$ is given by
     * \f[
     *   X = \frac{\varrho_0 \lambda}{\vec{v} \cdot \vec{a}} \left( \exp\left( \vec{v} \cdot \vec{a} \frac{l}{\lambda} \right) - 1 \right)
     * \f], where \f$ \varrho_0 \f$ is the density at the starting point.
     * 
     * If \f$ \vec{v} \cdot \vec{a} = 0 \f$, the calculation is just like with a homogeneous density:
     * \f[
     *   X = \varrho_0 l;
     * \f]
     */
    // clang-format on
    units::si::GrammageType IntegratedGrammage(
        geometry::Trajectory<geometry::Line> const& line,
        units::si::LengthType pTo,
        geometry::Vector<units::si::dimensionless_d> const& axis) const {
      auto const vDotA = line.NormalizedDirection().dot(axis).magnitude();
      auto const rhoStart = GetImplementation().GetMassDensity(line.GetR0());

      if (vDotA == 0) {
        return pTo * rhoStart;
      } else {
        return rhoStart * (fLambda / vDotA) * (exp(vDotA * pTo * fInvLambda) - 1);
      }
    }

    // clang-format off
    /**
     * For a (normalized) axis \f$ \vec{a} \f$, the length of a non-orthogonal line with (normalized)
     * direction \f$ \vec{v} \f$ corresponding to grammage \f$ X \f$ is given by
     * \f[
     *   l = \begin{cases}
     *   \frac{\lambda}{\vec{v} \cdot \vec{a}} \log\left(Y \right), & \text{if} Y :=  0 > 1 +
     *     \vec{v} \cdot \vec{a} \frac{X}{\rho_0 \lambda} 
     *   \infty & \text{else,}
     *   \end{cases}
     * \f] where \f$ \varrho_0 \f$ is the density at the starting point.
     * 
     * If \f$ \vec{v} \cdot \vec{a} = 0 \f$, the calculation is just like with a homogeneous density:
     * \f[
     *   l =  \frac{X}{\varrho_0}
     * \f]
     */
    // clang-format on
    units::si::LengthType ArclengthFromGrammage(
        geometry::Trajectory<corsika::geometry::Line> const& line,
        units::si::GrammageType pGrammage,
        geometry::Vector<units::si::dimensionless_d> const& axis) const {
      auto const vDotA = line.NormalizedDirection().dot(axis).magnitude();
      auto const rhoStart = GetImplementation().GetMassDensity(line.GetR0());

      if (vDotA == 0) {
        return pGrammage / rhoStart;
      } else {
        auto const logArg = pGrammage * fInvLambda * vDotA / rhoStart + 1;
        if (logArg > 0) {
          return fLambda / vDotA * log(logArg);
        } else {
          return std::numeric_limits<units::si::GrammageType::value_type>::infinity() *
                 units::si::meter;
        }
      }
    }

  public:
    BaseExponential(geometry::Point const& p0, units::si::MassDensityType rho,
                    units::si::LengthType lambda)
        : fRho0(rho)
        , fLambda(lambda)
        , fInvLambda(1 / lambda)
        , fP0(p0) {}
  };

} // namespace corsika::environment
#endif
