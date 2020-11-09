/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/process/tracking_line/Tracking.h>
#include <corsika/utl/quartic.h>

#include <type_traits>
#include <utility>

#include <fstream>

namespace corsika::process {

  namespace tracking_leapfrog_straight {

    typedef corsika::geometry::Vector<corsika::units::si::magnetic_flux_density_d>
        MagneticFieldVector;

    /**
     * \class Tracking
     *
     * The class tracking_leapfrog_straight::Tracking inherits from
     * tracking_line::Tracking and adds a (two-step) Leap-Frog
     * algorithms with two halve-steps and magnetic deflection.
     *
     * The two halve steps are implemented as two
     * `tracking_line::Tracking`s and all geometry intersections are,
     * thus, based on those two straight line elements.
     *
     * As a precaution for numerical instability, the steplength is
     * limited to correspond to a straight line distance to the next
     * volume intersection. In typical situations this leads to about
     * one full leap-frog step to the next volume boundary.
     *
     **/

    class Tracking : public tracking_line::Tracking {

    public:
      template <typename Particle>
      auto GetTrack(Particle& particle) {
        using namespace corsika::units::si;
        using namespace corsika::geometry;
        geometry::Vector<SpeedType::dimension_type> initialVelocity =
            particle.GetMomentum() / particle.GetEnergy() * corsika::units::constants::c;

        const Point initialPosition = particle.GetPosition();
        C8LOG_DEBUG(
            "TrackingB pid: {}"
            " , E = {} GeV",
            particle.GetPID(), particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("TrackingB pos: {}", initialPosition.GetCoordinates());
        C8LOG_DEBUG("TrackingB   E: {} GeV", particle.GetEnergy() / 1_GeV);
        C8LOG_DEBUG("TrackingB   p: {} GeV",
                    particle.GetMomentum().GetComponents() / 1_GeV);
        C8LOG_DEBUG("TrackingB   v: {} ", initialVelocity.GetComponents());

        typedef decltype(particle.GetNode()) node_type;
        const node_type volumeNode = particle.GetNode();
        auto magneticfield =
            volumeNode->GetModelProperties().GetMagneticField(initialPosition);

        // charge of the particle
        const int chargeNumber = particle.GetChargeNumber();
        const auto magnitudeB = magneticfield.GetNorm();
        C8LOG_DEBUG("field={} uT, chargeNumber={}, magnitudeB={} uT",
                    magneticfield.GetComponents() / 1_uT, chargeNumber, magnitudeB / 1_T);

        // we need to limit maximum step-length since we absolutely
        // need to follow strongly curved trajectories segment-wise,
        // at least if we don't employ concepts as "Helix
        // Trajectories" or similar
        auto const momentumVerticalMag =
            particle.GetMomentum() -
            particle.GetMomentum().parallelProjectionOnto(magneticfield);
        LengthType const gyroradius =
            (chargeNumber == 0 || magnitudeB == 0_T
                 ? std::numeric_limits<TimeType::value_type>::infinity() * 1_m
                 : momentumVerticalMag.norm() * 1_V /
                       (corsika::units::constants::c * abs(chargeNumber) * magnitudeB *
                        1_eV));
        const double maxRadians = 0.01;
        const LengthType steplimit = 2 * cos(maxRadians) * sin(maxRadians) * gyroradius;
        C8LOG_DEBUG("gyroradius {}, Steplimit: {}", gyroradius, steplimit);

        // calculate first halve step for "steplimit"
        const auto initialMomentum = particle.GetMomentum();
        const auto absMomentum = initialMomentum.norm();
        const auto absVelocity = initialVelocity.norm();
        const geometry::Vector<dimensionless_d> direction = initialVelocity.normalized();
        ;
        // check if particle is moving at all
        if (absVelocity * 1_s == 0_m) {
          return std::make_tuple(
              geometry::LineTrajectory(geometry::Line(initialPosition, initialVelocity),
                                       0_s),
              volumeNode);
        }

        // check, where the first halve-step direction has geometric intersections
        const auto [initialTrack, initialTrackNextVolume] =
            tracking_line::Tracking::GetTrack(particle);
        { [[maybe_unused]] auto& initialTrackNextVolume_dum = initialTrackNextVolume; }
        const auto initialTrackLength = initialTrack.GetLength(1);

        C8LOG_DEBUG("initialTrack(0)={}, initialTrack(1)={}, initialTrackLength={}",
                    initialTrack.GetPosition(0).GetCoordinates(),
                    initialTrack.GetPosition(1).GetCoordinates(), initialTrackLength);

        // avoid any intersections within first halve steplength
        LengthType firstHalveSteplength = std::min(steplimit, initialTrackLength) / 2;

        C8LOG_DEBUG("first halve step length {}, steplimit={}, initialTrackLength={}",
                    firstHalveSteplength, steplimit, initialTrackLength);
        // perform the first halve-step
        const Point position_mid = initialPosition + direction * firstHalveSteplength;
        const auto k = chargeNumber * corsika::units::constants::c * 1_eV /
                       (particle.GetMomentum().norm() * 1_V);
        const auto new_direction =
            direction + direction.cross(magneticfield) * firstHalveSteplength * 2 * k;
        const auto new_direction_norm = new_direction.norm(); // by design this is >1
        C8LOG_DEBUG("position_mid={}, new_direction={}, new_direction_norm={}",
                    position_mid.GetCoordinates(), new_direction.GetComponents(),
                    new_direction_norm);

        // check, where the second halve-step direction has geometric intersections
        particle.SetPosition(position_mid);
        particle.SetMomentum(new_direction * absMomentum);
        const auto [finalTrack, finalTrackNextVolume] =
            tracking_line::Tracking::GetTrack(particle);
        particle.SetPosition(initialPosition); // this is not nice...
        particle.SetMomentum(initialMomentum); // this is not nice...
        const auto finalTrackLength = finalTrack.GetLength(1);

        C8LOG_DEBUG("finalTrack(0)={}, finalTrack(1)={}, finalTrackLength={}",
                    finalTrack.GetPosition(0).GetCoordinates(),
                    finalTrack.GetPosition(1).GetCoordinates(), finalTrackLength);

        const LengthType secondLeapFrogLength = firstHalveSteplength * new_direction_norm;
        const LengthType secondHalveStepLength =
            std::min(secondLeapFrogLength, finalTrackLength);

        // perform the second halve-step
        const Point finalPosition = position_mid + new_direction * secondHalveStepLength;

        const LengthType totalStep = firstHalveSteplength + secondHalveStepLength;
        const auto delta_pos = finalPosition - initialPosition;
        const auto distance = delta_pos.norm();

        return std::make_tuple(
            geometry::LineTrajectory(
                geometry::Line(initialPosition,
                               (distance == 0_m ? initialVelocity
                                                : delta_pos.normalized() * absVelocity)),
                distance / absVelocity,  // straight distance
                totalStep / absVelocity, // bend distance
                initialVelocity,
                new_direction.normalized() * absVelocity), // trajectory
            (finalTrackLength > secondLeapFrogLength
                 ? volumeNode
                 : finalTrackNextVolume)); // next step volume
      }
    };

  } // namespace tracking_leapfrog_straight

} // namespace corsika::process
