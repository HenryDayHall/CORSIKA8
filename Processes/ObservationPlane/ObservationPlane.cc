/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/observation_plane/ObservationPlane.h>

#include <fstream>

using namespace corsika::process::observation_plane;
using namespace corsika::units::si;

ObservationPlane::ObservationPlane(geometry::Plane const& obsPlane,
                                   std::string const& filename, bool deleteOnHit)
    : plane_(obsPlane)
    , outputStream_(filename)
    , deleteOnHit_(deleteOnHit) {
  outputStream_ << "#PDG code, energy / eV, distance to center / m" << std::endl;
}

corsika::process::EProcessReturn ObservationPlane::DoContinuous(
    setup::Stack::ParticleType const& particle, setup::Trajectory const& trajectory) {
  TimeType const timeOfIntersection =
      (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) { return process::EProcessReturn::eOk; }

  if (plane_.IsAbove(trajectory.GetR0()) == plane_.IsAbove(trajectory.GetPosition(1))) {
    return process::EProcessReturn::eOk;
  }

  outputStream_ << static_cast<int>(particles::GetPDG(particle.GetPID())) << ' '
                << particle.GetEnergy() * (1 / 1_eV) << ' '
                << (trajectory.GetPosition(1) - plane_.GetCenter()).norm() / 1_m
                << std::endl;

  if (deleteOnHit_) {
    return process::EProcessReturn::eParticleAbsorbed;
  } else {
    return process::EProcessReturn::eOk;
  }
}

LengthType ObservationPlane::MaxStepLength(setup::Stack::ParticleType const&,
                                           setup::Trajectory const& trajectory) {
  TimeType const timeOfIntersection =
      (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) {
    return std::numeric_limits<double>::infinity() * 1_m;
  }

  auto const pointOfIntersection = trajectory.GetPosition(timeOfIntersection);
  return (trajectory.GetR0() - pointOfIntersection).norm() * 1.0001;
}
