/*
* (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
*
* This software is distributed under the terms of the GNU General Public
* Licence version 3 (GPL Version 3). See file LICENSE for a full version of
* the license.
*/
#pragma once

#include <corsika/modules/radio/propagators/FlatEarthPropagator.hpp>

namespace corsika {

 template <typename TEnvironment>
 inline FlatEarthPropagator<TEnvironment>::FlatEarthPropagator(TEnvironment const& env,
                                                                Point const& upperLimit, Point const& lowerLimit, LengthType const step)
     : RadioPropagator<FlatEarthPropagator, TEnvironment>(env)
         , upperLimit_(upperLimit)
         , lowerLimit_(lowerLimit)
         , step_(step)
         , inverseStep_(1 / step)
      {
                auto const maxHeight_ = upperLimit_.getCoordinates().getZ();
                auto const minHeight_ = lowerLimit_.getCoordinates().getZ();
                auto const minX_ = lowerLimit_.getCoordinates().getX();
                auto const minY_ = lowerLimit_.getCoordinates().getY();
                std::size_t const nBins_ = (maxHeight_ - minHeight_) * inverseStep_ + 1;
                rIndexTable_.reserve(nBins_);
                heightTable_.reserve(nBins_);
                integratedRIndexTable_.reserve(nBins_);

                // get the root coordinate system of this environment
                CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
                // get the universe for this environment
                auto const* const universe{Base::env_.getUniverse().get()};

                for (std::size_t i = 0; i < nBins_; i++) {
                  Point point_{rootCS, minX_, minY_, minHeight_ + i * step_};
                  auto const* const node{universe->getContainingNode(point_)};
                  auto const ri_ = node->getModelProperties().getRefractiveIndex(point_);
                  rIndexTable_.push_back(ri_);
                  auto const height_ = minHeight_ + i * step_;
                  heightTable_.push_back(height_);
                }

                double const stepOverMeter_{inverseStep_ * 1_m};
                auto const intRiZero_ = rIndexTable_.at(0) * stepOverMeter_;
                integratedRIndexTable_.push_back(intRiZero_);
                for (std::size_t i = 1; i < nBins_; i++) {
                  auto const intRi_ = integratedRIndexTable_[i-1] + integratedRIndexTable_[i] * stepOverMeter_;
                  integratedRIndexTable_.push_back(intRi_);
                }
            };

 template <typename TEnvironment>
 inline typename FlatEarthPropagator<TEnvironment>::SignalPathCollection
 FlatEarthPropagator<TEnvironment>::propagate(Point const& source, Point const& destination,
                                              [[maybe_unused]] LengthType const stepsize) {

   /**
    * This is a simple case of straight propagator where
    * tabulated values of refractive index are called assuming a flat earth.
    *
    */

   // these are used for the direction of emission and reception of signal at the antenna
   auto const emit_{(destination - source).normalized()};
   auto const receive_{-emit_};

   // the geometrical distance from the point of emission to an observer
   auto const distance_{(destination - source).getNorm()};

   // get the universe for this environment
   auto const* const universe{Base::env_.getUniverse().get()};

   // clear the refractive index vector and points deque for this signal propagation.
   rindex.clear();
   points.clear();

   // get and store the refractive index of the first point 'source'.
   std::size_t const indexSource_{static_cast<std::size_t>((source.getCoordinates().getZ() - heightTable_.front()) * inverseStep_ + 0.5)};
   auto const ri_source{rIndexTable_.at(indexSource_)};
   rindex.push_back(ri_source);
   points.push_back(source);

   // add the refractive index of last point 'destination' and store it.
   std::size_t const indexDestination_{static_cast<std::size_t>((destination.getCoordinates().getZ() - heightTable_.front()) * inverseStep_ + 0.5)};
   auto const ri_destination{rIndexTable_.at(indexDestination_)};
   rindex.push_back(ri_destination);
   points.push_back(destination);

   // compute the average refractive index.
   auto const averageRefractiveIndex_ = (ri_source + ri_destination) * 0.5;

   // compute the total time delay.
   TimeType const time = (integratedRIndexTable_.at(indexDestination_) - integratedRIndexTable_.at(indexSource_)) * (distance_ / constants::c);

   return {SignalPath(time, averageRefractiveIndex_, ri_source, ri_destination, emit_,
                      receive_, distance_, points)};

 } // END: propagate()

} // namespace corsika

