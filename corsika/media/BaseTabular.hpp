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
#include <corsika/framework/geometry/BaseTrajectory.hpp>

#include <limits>
#include <vector>

namespace corsika {

  /**
   * This class provides the grammage/length conversion functionality for
   * (locally) flat exponential atmospheres.
   */
  template <typename TDerived>
  class BaseTabular {

  public:
    BaseTabular(Point const& point, LengthType const referenceHeight,
                std::function<MassDensityType(LengthType)> const& rho,
                unsigned int const nBins, LengthType const deltaHeight);

    Point const& getAnchorPoint() const { return point_; }

  protected:
    auto const& getImplementation() const;

    MassDensityType getMassDensity(LengthType const height) const;

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
    GrammageType getIntegratedGrammage(BaseTrajectory const& line) const;

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
    LengthType getArclengthFromGrammage(BaseTrajectory const& line,
                                        GrammageType const grammage) const;

  private:
    unsigned int const nBins_;
    LengthType deltaHeight_;
    std::vector<MassDensityType> density_;
    Point const point_;
    LengthType const referenceHeight_;

  }; // class BaseTabular

} // namespace corsika

#include <corsika/detail/media/BaseTabular.inl>
