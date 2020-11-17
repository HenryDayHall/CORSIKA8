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
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/media/FlatExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>

namespace corsika {

  // clang-format off
  /**
   * The SlidingPlanarExponential models mass density as
   * \f[
   *   \varrho(r) = \varrho_0 \exp\left( \frac{|p_0 - r|}{\lambda} \right).
   * \f]
   * For grammage/length conversion, the density distribution is approximated as
   * locally flat at the starting point \f$ r_0 \f$ of the trajectory with the axis pointing
   * from \f$ p_0 \f$ to \f$ r_0 \f$.
   */
  // clang-format on

  template <typename T>
  class SlidingPlanarExponential : public BaseExponential<SlidingPlanarExponential<T>>,
                                   public T {
    NuclearComposition const nuclComp_;
    units::si::LengthType const referenceHeight_;

    using Base = BaseExponential<SlidingPlanarExponential<T>>;

  public:
    SlidingPlanarExponential(
        Point const& p0, units::si::MassDensityType rho0, units::si::LengthType lambda,
        NuclearComposition nuclComp,
        units::si::LengthType referenceHeight = units::si::LengthType::zero());

    units::si::MassDensityType getMassDensity(Point const& point) const override;

    NuclearComposition const& getNuclearComposition() const override;

    units::si::GrammageType integratedGrammage(Trajectory<Line> const& line,
                                               units::si::LengthType l) const override;

    units::si::LengthType arclengthFromGrammage(
        Trajectory<Line> const& line, units::si::GrammageType grammage) const override;
  };

} // namespace corsika

#include <corsika/detail/media/SlidingPlanarExponential.inl>
