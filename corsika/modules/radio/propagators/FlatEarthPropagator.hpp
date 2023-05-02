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
  * This class implements a simple propagator that uses
  * the straight-line (vector) between the particle
  * location and the antenna as the trajectory.
  *
  */
 template <typename TEnvironment>
 class FlatEarthPropagator final
     : public RadioPropagator<FlatEarthPropagator<TEnvironment>, TEnvironment> {

   using Base = RadioPropagator<FlatEarthPropagator<TEnvironment>, TEnvironment>;
   using SignalPathCollection = typename Base::SignalPathCollection;

 public:
   /**
    * Construct a new FlatEarthPropagator with a given environment.
    *
    */
   FlatEarthPropagator(TEnvironment const& env, Point const& upperLimit, Point const& lowerLimit, LengthType const step);

   /**
    * Return the collection of paths from `source` to `destination`.
    * Hence, the signal propagated from the
    * emission point to the antenna location.
    *
    */
   SignalPathCollection propagate(Point const& source, Point const& destination,
                                  LengthType const stepsize);

 private:
   Point const upperLimit_;
   Point const lowerLimit_;
   LengthType const step_;
   InverseLengthType const inverseStep_;
   std::vector<double> rIndexTable_;
   std::vector<double> integratedRIndexTable_;
   std::vector<LengthType> heightTable_;
   std::deque<Point> points;
   std::vector<double> rindex;

 }; // End: FlatEarthPropagator

} // namespace corsika

#include <corsika/detail/modules/radio/propagators/FlatEarthPropagator.inl>