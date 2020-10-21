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
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

#include <limits>

namespace corsika {

  /**
   * This class provides the grammage/length conversion functionality for
   * (locally) flat exponential atmospheres.
   */
  template <class TDerived>
  class BaseExponential {

  public:
    BaseExponential(Point const& vP0, MassDensityType vRho, LengthType vLambda)
        : fRho0(vRho)
        , fLambda(vLambda)
        , fInvLambda(1 / vLambda)
        , fP0(vP0) {}

  protected:
    MassDensityType const fRho0;
    LengthType const fLambda;
    InverseLengthType const fInvLambda;
    Point const fP0;

    auto const& GetImplementation() const;

    // clang-format off
    /**
     * For a (normalized) axis \f$ \vec{a} \f$, the grammage along a non-orthogonal line with (normalized)
     * direction \f$ \vec{u} \f$ is given by
     * \f[
     *   X = \frac{\varrho_0 \lambda}{\vec{u} \cdot \vec{a}} \left( \exp\left( \vec{u} \cdot \vec{a} \frac{l}{\lambda} \right) - 1 \right)
     * \f], where \f$ \varrho_0 \f$ is the density at the starting point.
     * 
     * If \f$ \vec{u} \cdot \vec{a} = 0 \f$, the calculation is just like with a homogeneous density:
     * \f[
     *   X = \varrho_0 l;
     * \f]
     */
    // clang-format on
    GrammageType IntegratedGrammage(Trajectory<Line> const& vLine, LengthType vL,
                                    Vector<dimensionless_d> const& vAxis) const;

    // clang-format off
    /**
     * For a (normalized) axis \f$ \vec{a} \f$, the length of a non-orthogonal line with (normalized)
     * direction \f$ \vec{u} \f$ corresponding to grammage \f$ X \f$ is given by
     * \f[
     *   l = \begin{cases}
     *   \frac{\lambda}{\vec{u} \cdot \vec{a}} \log\left(Y \right), & \text{if} Y :=  0 > 1 +
     *     \vec{u} \cdot \vec{a} \frac{X}{\rho_0 \lambda} 
     *   \infty & \text{else,}
     *   \end{cases}
     * \f] where \f$ \varrho_0 \f$ is the density at the starting point.
     * 
     * If \f$ \vec{u} \cdot \vec{a} = 0 \f$, the calculation is just like with a homogeneous density:
     * \f[
     *   l =  \frac{X}{\varrho_0}
     * \f]
     */
    // clang-format on
    LengthType ArclengthFromGrammage(Trajectory<Line> const& vLine,
                                     GrammageType vGrammage,
                                     Vector<dimensionless_d> const& vAxis) const;
  };

} // namespace corsika

#include <corsika/detail/media/BaseExponential.inl>
