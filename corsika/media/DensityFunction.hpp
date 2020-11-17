/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/media/LinearApproximationIntegrator.hpp>

namespace corsika {

  template <class TDerivableRho,
            template <typename> class TIntegrator = LinearApproximationIntegrator>
  class DensityFunction
      : public TIntegrator<DensityFunction<TDerivableRho, TIntegrator>> {
    friend class TIntegrator<DensityFunction<TDerivableRho, TIntegrator>>;

    TDerivableRho rho_; //!< functor for density

  public:
    DensityFunction(TDerivableRho rho)
        : rho_(rho) {}

    MassDensityType evaluateAt(corsika::Point const& p) const { return rho_(p); }
  };

} // namespace corsika
