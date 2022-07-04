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
   * This class implements a basic propagator that uses
   * the straight-line (vector) between the particle
   * location and the antenna as the trajectory.
   *
   * This is what is used in ZHAireS and CoREAS in C7.
   */
  template <typename TEnvironment>
  class StraightPropagator final
      : public RadioPropagator<StraightPropagator<TEnvironment>, TEnvironment> {

    using Base = RadioPropagator<StraightPropagator<TEnvironment>, TEnvironment>;
    using SignalPathCollection = typename Base::SignalPathCollection;

  public:
    /**
     * Construct a new StraightPropagator with a given environment.
     *
     */
    StraightPropagator(TEnvironment const& env);

    /**
     * Return the collection of paths from `start` to `end`.
     * or from 'source' which is the emission point to 'destination'
     * which is the location of the antenna
     */
    SignalPathCollection propagate(Point const& source, Point const& destination,
                                   LengthType const stepsize) const;

  }; // End: StraightPropagator

} // namespace corsika

#include <corsika/detail/modules/radio/propagators/StraightPropagator.inl>