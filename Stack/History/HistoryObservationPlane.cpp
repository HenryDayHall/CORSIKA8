/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/logging/Logging.h>
#include <corsika/stack/history/HistoryObservationPlane.hpp>

#include <boost/histogram/ostream.hpp>

#include <fstream>
#include <iostream>

using namespace corsika::units::si;
using namespace corsika::history;
using namespace corsika;

HistoryObservationPlane::HistoryObservationPlane(setup::Stack const& stack,
                                                 geometry::Plane const& obsPlane,
                                                 bool deleteOnHit)
    : stack_{stack}
    , plane_{obsPlane}
    , deleteOnHit_{deleteOnHit} {}

corsika::process::EProcessReturn HistoryObservationPlane::DoContinuous(
    setup::Stack::ParticleType const& particle, setup::Trajectory const& trajectory) {
  TimeType const timeOfIntersection =
      (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) { return process::EProcessReturn::eOk; }

  if (plane_.IsAbove(trajectory.GetR0()) == plane_.IsAbove(trajectory.GetPosition(1))) {
    return process::EProcessReturn::eOk;
  }

  C8LOG_DEBUG(fmt::format("HistoryObservationPlane: Particle detected: pid={}",
                          particle.GetPID()));

  auto const pid = particle.GetPID();
  if (particles::IsMuon(pid)) { fillHistoryHistogram(particle); }

  if (deleteOnHit_) {
    return process::EProcessReturn::eParticleAbsorbed;
  } else {
    return process::EProcessReturn::eOk;
  }
}

LengthType HistoryObservationPlane::MaxStepLength(setup::Stack::ParticleType const&,
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

void HistoryObservationPlane::fillHistoryHistogram(
    setup::Stack::ParticleType const& muon) {
  //  double const muonEnergy = muon.GetEnergy() / 1_eV;

  // auto parent = stack_.begin() + muon.GetEvent()->projectileIndex();
  Event* event = muon.GetEvent().get();

  int intCounter = 0;
  while (event) {
    event = event->parentEvent().get();
    intCounter++;
  }
  histogram_(intCounter);
}

void HistoryObservationPlane::print() { std::cout << histogram_ << std::endl; }
