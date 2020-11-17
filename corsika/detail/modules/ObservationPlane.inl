/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/ObservationPlane.hpp>

#include <fstream>

namespace corsika::observation_plane {

  ObservationPlane::ObservationPlane(corsika::Plane const& obsPlane,
                                     std::string const& filename, bool deleteOnHit)
      : plane_(obsPlane)
      , outputStream_(filename)
      , deleteOnHit_(deleteOnHit) {
    outputStream_ << "#PDG code, energy / eV, distance to center / m" << std::endl;
  }

  corsika::EProcessReturn ObservationPlane::DoContinuous(
      corsika::setup::Stack::ParticleType const& particle,
      corsika::setup::Trajectory const& trajectory) {
    TimeType const timeOfIntersection =
        (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
        trajectory.GetV0().dot(plane_.GetNormal());

    if (timeOfIntersection < TimeType::zero()) { return corsika::EProcessReturn::eOk; }

    if (plane_.IsAbove(trajectory.GetR0()) == plane_.IsAbove(trajectory.GetPosition(1))) {
      return corsika::EProcessReturn::eOk;
    }

    outputStream_ << static_cast<int>(corsika::PDG(particle.GetPID())) << ' '
                  << particle.GetEnergy() * (1 / 1_eV) << ' '
                  << (trajectory.GetPosition(1) - plane_.GetCenter()).norm() / 1_m
                  << std::endl;

    if (deleteOnHit_) {
      return corsika::EProcessReturn::eParticleAbsorbed;
    } else {
      return corsika::EProcessReturn::eOk;
    }
  }

  corsika::LengthType ObservationPlane::MaxStepLength(
      corsika::setup::Stack::ParticleType const&,
      corsika::setup::Trajectory const& trajectory) {

    TimeType const timeOfIntersection =
        (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
        trajectory.GetV0().dot(plane_.GetNormal());

    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }

    auto const pointOfIntersection = trajectory.GetPosition(timeOfIntersection);
    return (trajectory.GetR0() - pointOfIntersection).norm() * 1.0001;
  }

} // namespace corsika::observation_plane
