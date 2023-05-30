/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

namespace corsika {

  /**
   * This class implements a dummy propagator that uses
   * the straight-line (vector) between the particle
   * location and the antenna as the trajectory.
   * It is intended mainly for fast testing as it only
   * works with 2 points in a uniform refractive index
   * atmospheric profile.
   *
   */
  template <typename TEnvironment>
  class DummyTestPropagator final
      : public RadioPropagator<DummyTestPropagator<TEnvironment>, TEnvironment> {

    using Base = RadioPropagator<DummyTestPropagator<TEnvironment>, TEnvironment>;
    using SignalPathCollection = typename Base::SignalPathCollection;

  public:
    /**
     * Construct a new SimplePropagator with a given environment.
     *
     */
    DummyTestPropagator(TEnvironment const& env);

    /**
     * Return the collection of paths from `source` to `destination`.
     * Hence, the signal propagated from the
     * emission point to the antenna location.
     *
     */
    SignalPathCollection propagate(Point const& source, Point const& destination,
                                   LengthType const stepsize);

  private:
      std::deque<Point> points;
      std::vector<double> rindex;

  }; // End: SimplePropagator

template <typename TEnvironment>
  DummyTestPropagator<TEnvironment>
make_dummy_test_radio_propagator(TEnvironment const& env){
    return DummyTestPropagator<TEnvironment>(env);

}


} // namespace corsika

#include <corsika/detail/modules/radio/propagators/DummyTestPropagator.inl>