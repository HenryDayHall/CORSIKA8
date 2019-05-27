
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
    SlidingPlanarExponential(geometry::Point const& vP0, units::si::MassDensityType vRho,
                             units::si::LengthType vLambda, NuclearComposition vNuclComp)
        : Base(vP0, vRho, vLambda)
        , fNuclComp(vNuclComp) {}

    units::si::MassDensityType GetMassDensity(
        geometry::Point const& vP) const override {
      auto const height = (vP - Base::fP0).norm();
      return Base::fRho0 * exp(Base::fInvLambda * height);
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    units::si::GrammageType IntegratedGrammage(
        geometry::Trajectory<geometry::Line> const& vLine,
        units::si::LengthType vL) const override {
      auto const axis = (vLine.GetR0() - Base::fP0).normalized();
      return Base::IntegratedGrammage(vLine, vL, axis);
    }

    units::si::LengthType ArclengthFromGrammage(
        geometry::Trajectory<geometry::Line> const& vLine,
        units::si::GrammageType vGrammage) const override {
      auto const axis = (vLine.GetR0() - Base::fP0).normalized();
      return Base::ArclengthFromGrammage(vLine, vGrammage, axis);
    }
  };

} // namespace corsika::environment
#endif
