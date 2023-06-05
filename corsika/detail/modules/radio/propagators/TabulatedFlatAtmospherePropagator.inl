/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/propagators/TabulatedFlatAtmospherePropagator.hpp>

namespace corsika {

  template <typename TEnvironment>
  inline TabulatedFlatAtmospherePropagator<
      TEnvironment>::TabulatedFlatAtmospherePropagator(TEnvironment const& env,
                                                       Point const& upperLimit,
                                                       Point const& lowerLimit,
                                                       LengthType const step)
      : RadioPropagator<TabulatedFlatAtmospherePropagator, TEnvironment>(env)
      , upperLimit_(upperLimit)
      , lowerLimit_(lowerLimit)
      , step_(step)
      , inverseStep_(1 / step) {
    auto const maxHeight_ = upperLimit_.getCoordinates().getZ();
    auto const minHeight_ = lowerLimit_.getCoordinates().getZ();
    auto const minX_ = lowerLimit_.getCoordinates().getX();
    auto const minY_ = lowerLimit_.getCoordinates().getY();
    std::size_t const nBins_ = (maxHeight_ - minHeight_) * inverseStep_ + 1;
    refractivityTable_.reserve(nBins_);
    heightTable_.reserve(nBins_);
    integratedRefractivityTable_.reserve(nBins_);

    // get the root coordinate system of this environment
    CoordinateSystemPtr const& rootCS = env.getCoordinateSystem();
    // get the universe for this environment
    auto const* const universe{Base::env_.getUniverse().get()};

    for (std::size_t i = 0; i < nBins_; i++) {
      Point point_{rootCS, minX_, minY_, minHeight_ + i * step_};
      auto const* const node{universe->getContainingNode(point_)};
      auto const ri_ = node->getModelProperties().getRefractiveIndex(point_);
      refractivityTable_.push_back(ri_ - 1);
      auto const height_ = minHeight_ + i * step_;
      heightTable_.push_back(height_);
    }

    double const stepOverMeter_{inverseStep_ * 1_m};
    auto const intRerfZero_ = refractivityTable_.at(0) * stepOverMeter_;
    integratedRefractivityTable_.push_back(intRerfZero_);
    for (std::size_t i = 1; i < nBins_; i++) {
      auto const intRefrI_ =
          integratedRefractivityTable_[i - 1] + refractivityTable_[i] * stepOverMeter_;
      integratedRefractivityTable_.push_back(intRefrI_);
    }
  };

  template <typename TEnvironment>
  template <typename Particle>
  inline typename TabulatedFlatAtmospherePropagator<TEnvironment>::SignalPathCollection
  TabulatedFlatAtmospherePropagator<TEnvironment>::propagate(Particle const& particle, Point const& source,
                                                             Point const& destination) {

    /**
     * This is a simple case of straight propagator where
     * tabulated values of refractive index are called assuming
     * a flat atmosphere.
     *
     */

    // these are used for the direction of emission and reception of signal at the antenna
    auto const emit_{(destination - source).normalized()};
    auto const receive_{-emit_};

    // the geometrical distance from the point of emission to an observer
    auto const distance_{(destination - source).getNorm()};

    // clear the refractive index vector and points deque for this signal propagation.
    rindex.clear();
    points.clear();

    // get and store the refractive index of the first point 'source'.
    std::size_t const indexSource_{static_cast<std::size_t>(
        (source.getCoordinates().getZ() - heightTable_.front()) * inverseStep_ +
        0.5)}; // ToDo: this does no interpolation for particles in ground, it just stops
               // on the surface.
    auto const ri_source{refractivityTable_.at(indexSource_) + 1};
    rindex.push_back(ri_source);
    points.push_back(source);

    // add the refractive index of last point 'destination' and store it.
    std::size_t const indexDestination_{static_cast<std::size_t>(
        (destination.getCoordinates().getZ() - heightTable_.front()) * inverseStep_ +
        0.5)};
    auto const ri_destination{refractivityTable_.at(indexDestination_) + 1};
    rindex.push_back(ri_destination);
    points.push_back(destination);

    auto const height_ =
        (heightTable_.at(indexSource_) - heightTable_.at(indexDestination_)) / 1_m;
    // compute the average refractive index.
    auto const averageRefractiveIndex_ = (ri_source + ri_destination) * 0.5;

    // compute the total time delay.
    TimeType const time = (1 + (integratedRefractivityTable_.at(indexSource_) -
                                integratedRefractivityTable_.at(indexDestination_)) /
                                   height_) *
                          (distance_ / constants::c);

    return {SignalPath(time, averageRefractiveIndex_, ri_source, ri_destination, emit_,
                       receive_, distance_, points)};

  } // END: propagate()

} // namespace corsika
