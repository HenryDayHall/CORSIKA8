/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_SlidingPlanarExponential_h_
#define _include_Environment_SlidingPlanarExponential_h_

#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

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
   //clang-format on
   
  template <class T>
  class SlidingPlanarExponential : public BaseExponential<SlidingPlanarExponential<T>>,
                                   public T {
    NuclearComposition const fNuclComp;

    using Base = BaseExponential<SlidingPlanarExponential<T>>;

  public:
    SlidingPlanarExponential(geometry::Point const& p0, units::si::MassDensityType rho,
                             units::si::LengthType lambda, NuclearComposition nuclComp)
        : Base(p0, rho, lambda)
        , fNuclComp(nuclComp) {}

    corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const& p) const override {
      auto const height = (p - Base::fP0).norm();
      return Base::fRho0 * exp(Base::fInvLambda * height);
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::LengthType pTo) const override {
      auto const axis = (line.GetR0() - Base::fP0).normalized();
      return Base::IntegratedGrammage(line, pTo, axis);
    }

    corsika::units::si::LengthType ArclengthFromGrammage(
        corsika::geometry::Trajectory<corsika::geometry::Line> const& line,
        corsika::units::si::GrammageType pGrammage) const override {
      auto const axis = (line.GetR0() - Base::fP0).normalized();
      return Base::ArclengthFromGrammage(line, pGrammage, axis);
    }
  };

} // namespace corsika::environment
#endif
