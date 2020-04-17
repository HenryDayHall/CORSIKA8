/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/LinearApproximationIntegrator.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>

namespace corsika {

  template <class TDerivableRho, template <typename> class TIntegrator = LinearApproximationIntegrator>
  class DensityFunction: public TIntegrator<DensityFunction<TDerivableRho, TIntegrator>>
  {
    friend class TIntegrator<DensityFunction<TDerivableRho, TIntegrator>>;

    TDerivableRho fRho; //!< functor for density

  public:

    DensityFunction(TDerivableRho rho)
        : fRho(rho) {}

    corsika::units::si::MassDensityType EvaluateAt(
        corsika::Point const& p) const {
      return fRho(p);
    }
  };

} // namespace corsika::environment

