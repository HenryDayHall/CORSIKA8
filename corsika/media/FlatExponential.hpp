/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/media/BaseExponential.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/BaseTrajectory.hpp>

namespace corsika {

  // clang-format off
  /**
   * flat exponential density distribution with
   * \f[
   *  \varrho(r) = \varrho_0 \exp\left( \frac{1}{\lambda} (r - p) \cdot
   *    \vec{a} \right).
   * \f]
   * \f$ \vec{a} \f$ denotes the axis and should be normalized to avoid degeneracy
   * with the scale parameter \f$ \lambda \f$.
   */
  // clang-format on
  template <typename T>
  class FlatExponential : public BaseExponential<FlatExponential<T>>, public T {
    using base_type = BaseExponential<FlatExponential<T>>;

  public:
    FlatExponential(Point const& point, Vector<dimensionless_d> const& axis,
                    MassDensityType rho, LengthType lambda,
                    NuclearComposition const& nuclComp);

    MassDensityType getMassDensity(Point const& point) const override;

    NuclearComposition const& getNuclearComposition() const override;

    GrammageType getIntegratedGrammage(BaseTrajectory const& line,
                                       LengthType to) const override;

    LengthType getArclengthFromGrammage(BaseTrajectory const& line,
                                        GrammageType grammage) const override;

  private:
    DirectionVector const axis_;
    NuclearComposition const nuclComp_;
  };

} // namespace corsika

#include <corsika/detail/media/FlatExponential.inl>
