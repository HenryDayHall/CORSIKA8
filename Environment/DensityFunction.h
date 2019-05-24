/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_environment_DensityFunction_h_
#define _include_environment_DensityFunction_h_

#include <corsika/environment/LinearApproximationIntegrator.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>

namespace corsika::environment {

  template <class TDerivableRho,
            template <typename> class TIntegrator = LinearApproximationIntegrator>
  class DensityFunction
      : public TIntegrator<DensityFunction<TDerivableRho, TIntegrator>> {
    friend class TIntegrator<DensityFunction<TDerivableRho, TIntegrator>>;

    TDerivableRho fRho; //!< functor for density

  public:
    DensityFunction(TDerivableRho rho)
        : fRho(rho) {}

    corsika::units::si::MassDensityType EvaluateAt(
        corsika::geometry::Point const& p) const {
      return fRho(p);
    }
  };
} // namespace corsika::environment

#endif
