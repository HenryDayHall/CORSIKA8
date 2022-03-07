/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/propagators/SimplePropagator.hpp>

namespace corsika {

  template <typename TEnvironment>
  inline SimplePropagator<TEnvironment>::SimplePropagator(const TEnvironment& env)
      : RadioPropagator<SimplePropagator, TEnvironment>(env){};

  template <typename TEnvironment>
  inline typename SimplePropagator<TEnvironment>::SignalPathCollection
  SimplePropagator<TEnvironment>::propagate(const Point& source, const Point& destination,
                                            const LengthType stepsize) const {

    /**
     * This is the simplest case of straight propagator
     * where no integration takes place.
     * This can be used for fast tests and checks of the radio module.
     *
     */

    // these are used for the direction of emission and reception of signal at the antenna
    auto emit_{(destination - source).normalized()};
    auto receive_{-emit_};

    // the geometrical distance from the point of emission to an observer
    auto distance_{(destination - source).getNorm()};

    // get the universe for this environment
    auto const* const universe{Base::env_.getUniverse().get()};

    // the points that consist the signal path (source & destination).
    std::deque<Point> points;

    // store value of the refractive index at points.
    std::vector<double> rindex;
    rindex.reserve(2);

    // get and store the refractive index of the first point 'source'.
    auto const* nodeSource{universe->getContainingNode(source)};
    auto const ri_source{nodeSource->getModelProperties().getRefractiveIndex(source)};
    rindex.push_back(ri_source);
    points.push_back(source);

    // add the refractive index of last point 'destination' and store it.
    auto const* node{universe->getContainingNode(destination)};
    auto const ri_destination{node->getModelProperties().getRefractiveIndex(destination)};
    rindex.push_back(ri_destination);
    points.push_back(destination);

    // compute the average refractive index.
    auto averageRefractiveIndex_ = (ri_source + ri_destination) / 2;

    // compute the total time delay.
    TimeType time = averageRefractiveIndex_ * (distance_ / constants::c);

    return {SignalPath(time, averageRefractiveIndex_, ri_source, ri_destination, emit_,
                       receive_, distance_, points)};

  } // END: propagate()

} // namespace corsika
