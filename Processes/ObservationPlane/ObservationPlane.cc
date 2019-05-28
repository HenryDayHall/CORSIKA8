/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/observation_plane/ObservationPlane.h>

#include <fstream>

using namespace corsika::process::observation_plane;
using namespace corsika::units::si;

ObservationPlane::ObservationPlane(geometry::Plane const& vObsPlane,
                                   std::string const& vFilename)
    : fObsPlane(vObsPlane)
    , fOutputStream(vFilename) {
  fOutputStream << "#PDG code, energy / eV, distance to center / m" << std::endl;
}

corsika::process::EProcessReturn ObservationPlane::DoContinuous(
    setup::Stack::ParticleType const& vParticle, setup::Trajectory const& vTrajectory) {

  if (fObsPlane.IsAbove(vTrajectory.GetPosition(1.0001))) {
    return process::EProcessReturn::eOk;
  }

  fOutputStream << static_cast<int>(particles::GetPDG(vParticle.GetPID())) << ' '
                << vParticle.GetEnergy() * (1 / 1_eV) << ' '
                << (vTrajectory.GetPosition(1) - fObsPlane.GetCenter()).norm() / 1_m
                << std::endl;

  return process::EProcessReturn::eParticleAbsorbed;
}

LengthType ObservationPlane::MaxStepLength(setup::Stack::ParticleType const&,
                                           setup::Trajectory const& vTrajectory) {
  if (!fObsPlane.IsAbove(vTrajectory.GetR0())) {
    return std::numeric_limits<double>::infinity() * 1_m;
  }

  auto const pointOfIntersection = vTrajectory.GetPosition(
      (fObsPlane.GetCenter() - vTrajectory.GetR0()).dot(fObsPlane.GetNormal()) /
      vTrajectory.GetV0().dot(fObsPlane.GetNormal()));
  return (vTrajectory.GetR0() - pointOfIntersection).norm();
}
