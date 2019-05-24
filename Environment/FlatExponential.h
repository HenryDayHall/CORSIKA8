/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_Environment_FlatExponential_h_
#define _include_Environment_FlatExponential_h_

#include <corsika/environment/BaseExponential.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::environment {

  //clang-format off
  /**
   * flat exponential density distribution with
   * \f[
   *  \varrho(r) = \varrho_0 \exp\left( \frac{1}{\lambda} (r - p) \cdot
   *    \vec{a} \right).
   * \f]
   * \f$ \vec{a} \f$ denotes the axis and should be normalized to avoid degeneracy
   * with the scale parameter \f$ \lambda \f$.
   */
  //clang-format on
  template <class T>
  class FlatExponential : public BaseExponential<FlatExponential<T>>, public T {
    geometry::Vector<units::si::dimensionless_d> const fAxis;
    NuclearComposition const fNuclComp;

    using Base = BaseExponential<FlatExponential<T>>;

  public:
    FlatExponential(geometry::Point const& vP0,
                    geometry::Vector<units::si::dimensionless_d> const& vAxis,
                    units::si::MassDensityType vRho, units::si::LengthType vLambda,
                    NuclearComposition vNuclComp)
        : Base(vP0, vRho, vLambda)
        , fAxis(vAxis)
        , fNuclComp(vNuclComp) {}

    units::si::MassDensityType GetMassDensity(geometry::Point const& vP) const override {
      return Base::fRho0 * exp(Base::fInvLambda * (vP - Base::fP0).dot(fAxis));
    }

    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    units::si::GrammageType IntegratedGrammage(
        geometry::Trajectory<geometry::Line> const& vLine,
        units::si::LengthType vTo) const override {
      return Base::IntegratedGrammage(vLine, vTo, fAxis);
    }

    units::si::LengthType ArclengthFromGrammage(
        geometry::Trajectory<geometry::Line> const& vLine,
        units::si::GrammageType vGrammage) const override {
      return Base::ArclengthFromGrammage(vLine, vGrammage, fAxis);
    }
  };
} // namespace corsika::environment
#endif
