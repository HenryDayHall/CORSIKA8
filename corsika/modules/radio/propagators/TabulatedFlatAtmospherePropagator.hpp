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
  * This class implements a tabulated propagator that approximates
  * the Earth's atmosphere as flat. Signal propagation is rectilinear
  * and this is intended to be used for vertical showers
  * (<60 degrees zenith angle) for fast simulations.
  *
  */
 template <typename TEnvironment>
 class TabulatedFlatAtmospherePropagator final
     : public RadioPropagator<TabulatedFlatAtmospherePropagator<TEnvironment>, TEnvironment> {

   using Base = RadioPropagator<TabulatedFlatAtmospherePropagator<TEnvironment>, TEnvironment>;
   using SignalPathCollection = typename Base::SignalPathCollection;

 public:
   /**
    * Construct a new FlatEarthPropagator with a given environment.
    *
    */
   TabulatedFlatAtmospherePropagator(TEnvironment const& env, Point const& upperLimit, Point const& lowerLimit,
                       LengthType const step);

   /**
    * Return the collection of paths from `source` to `destination`.
    * Hence, the signal propagated from the
    * emission point to the antenna location.
    *
    */
   SignalPathCollection propagate(Point const& source, Point const& destination);

 private:
   Point const upperLimit_;
   Point const lowerLimit_;
   LengthType const step_;
   InverseLengthType const inverseStep_;
   std::vector<double> refractivityTable_;
   std::vector<double> integratedRefractivityTable_;
   std::vector<LengthType> heightTable_;
   std::deque<Point> points;
   std::vector<double> rindex;

 }; // End: FlatEarthPropagator

} // namespace corsika

#include <corsika/detail/modules/radio/propagators/TabulatedFlatAtmospherePropagator.inl>