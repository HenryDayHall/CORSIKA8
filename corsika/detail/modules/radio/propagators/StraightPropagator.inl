/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/propagators/StraightPropagator.hpp>

namespace corsika {

    template <typename TEnvironment>
    // TODO: maybe the constructor doesn't take any arguments for the environment (?)
    inline StraightPropagator<TEnvironment>::StraightPropagator(const TEnvironment &env)
            : RadioPropagator<StraightPropagator, TEnvironment>(env){};

    template <typename TEnvironment>
    inline typename StraightPropagator<TEnvironment>::SignalPathCollection StraightPropagator<TEnvironment>::propagate(
            const Point &source, const Point &destination, const LengthType stepsize) const {

        /**
         * get the normalized (unit) vector from `source` to `destination'.
         * this is also the `emit` and `receive` vectors in the SignalPath class.
         * in this case emit and receive unit vectors should be the same
         * so they are both called direction
         */

        // these are used for the direction of emission and reception of signal at the antenna
        auto emit_{(destination - source).normalized()};
        auto receive_{ - emit_};

        // the distance from the point of emission to an observer
        auto distance_ {(destination - source).getNorm()};

        try {
            if (stepsize <= 0.5 * distance_) {

                // "step" is the direction vector with length `stepsize`
                auto step{emit_ * stepsize};

                // calculate the number of points (roughly) for the numerical integration
                auto n_points{(destination - source).getNorm() / stepsize};

                // get the universe for this environment
                auto const* const universe{Base::env_.getUniverse().get()};

                // the points that we build along the way for the numerical integration
                std::deque<Point> points;

                // store value of the refractive index at points
                std::vector<double> rindex;
                rindex.reserve(n_points);

                // get and store the refractive index of the first point 'source'
                auto const* nodeSource{universe->getContainingNode(source)};
                auto const ri_source{
                        nodeSource->getModelProperties().getRefractiveIndex(source)};
                rindex.push_back(ri_source);
                points.push_back(source);

                // loop from `source` to `destination` to store values before Simpson's rule.
                // this loop skips the last point 'destination' and "misses" the extra point
                for (auto point = source + step;
                     (point - destination).getNorm() > 0.6 * stepsize; point = point + step) {

                    // get the environment node at this specific 'point'
                    auto const* node{universe->getContainingNode(point)};

                    // get the associated refractivity at 'point'
                    auto const refractive_index{
                            node->getModelProperties().getRefractiveIndex(point)};
                    //         auto const refractive_index{1.000327};
                    rindex.push_back(refractive_index);

                    // add this 'point' to our deque collection
                    points.push_back(point);
                }

                // Get the extra points that the for loop misses until the destination
                auto const extrapoint_ {points.back() + step};

                // add the refractive index of last point 'destination' and store it
                auto const* node{universe->getContainingNode(destination)};
                auto const ri_destination{node->getModelProperties().getRefractiveIndex(destination)};
                //      auto const ri_destination{1.000327};
                rindex.push_back(ri_destination);
                points.push_back(destination);

                auto N = rindex.size();
                std::size_t index = 0;
                double sum = rindex.at(index);
                auto refra_ = rindex.at(index);
                TimeType time {0_s};

                if ((N-1) % 2 == 0) {
                    // Apply the standard Simpson's rule
                    auto h = ((destination - source).getNorm()) / (N - 1);

                    for (std::size_t index = 1; index < (N - 1); index += 2) {
                        sum += 4 * rindex.at(index);
                        refra_ += rindex.at(index);
                    }
                    for (std::size_t index = 2; index < (N - 1); index += 2) {
                        sum += 2 * rindex.at(index);
                        refra_ += rindex.at(index);
                    }
                    index = N - 1;
                    sum = sum + rindex.at(index);
                    refra_ += rindex.at(index);

                    // compute the total time delay.
                    time = sum * (h / (3 * constants::c));
                } else {
                    // Apply Simpson's rule for one "extra" point and then subtract the difference
                    points.pop_back();
                    rindex.pop_back();
                    auto const* node{universe->getContainingNode(extrapoint_)};
                    auto const ri_extrapoint{node->getModelProperties().getRefractiveIndex(extrapoint_)};
                    rindex.push_back(ri_extrapoint);
                    points.push_back(extrapoint_);
                    auto const extrapoint2_ {extrapoint_ + step};
                    auto const* node2{universe->getContainingNode(extrapoint2_)};
                    auto const ri_extrapoint2{node2->getModelProperties().getRefractiveIndex(extrapoint2_)};
                    rindex.push_back(ri_extrapoint2);
                    points.push_back(extrapoint2_);
                    N = rindex.size();
                    auto h = ((extrapoint2_ - source).getNorm()) / (N - 1);
                    for (std::size_t index = 1; index < (N - 1); index += 2) {
                        sum += 4 * rindex.at(index);
                        refra_ += rindex.at(index);
                    }
                    for (std::size_t index = 2; index < (N - 1); index += 2) {
                        sum += 2 * rindex.at(index);
                        refra_ += rindex.at(index);
                    }
                    index = N - 1;
                    sum = sum + rindex.at(index);
                    refra_ += rindex.at(index);

                    // compute the total time delay including the correction
                    time = sum * (h / (3 * constants::c)) - (ri_extrapoint2 * ((extrapoint2_ - destination).getNorm()) / constants::c);
                }

                // uncomment the following if you want to skip the integration for fast tests
                //TimeType time = ri_destination * (distance_ / constants::c);

                // compute the average refractive index.
                auto averageRefractiveIndex_ = refra_ / N;

                return {SignalPath(time, averageRefractiveIndex_, ri_source, ri_destination,
                                   emit_, receive_, distance_, points)};
            } else {
                throw stepsize;
            }
        } catch (const LengthType& s) {
            CORSIKA_LOG_ERROR("Please choose a smaller stepsize for the numerical integration");
        }

    } // END: propagate()

} // namespace corsika
