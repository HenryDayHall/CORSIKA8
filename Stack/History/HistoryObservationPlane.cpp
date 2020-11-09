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

#include <iomanip>
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
    (plane_.GetCenter() - trajectory.GetLine().GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetLine().GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) { return process::EProcessReturn::eOk; }

  if (plane_.IsAbove(trajectory.GetLine().GetR0()) == plane_.IsAbove(trajectory.GetPosition(1))) {
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
      (plane_.GetCenter() - trajectory.GetLine().GetR0()).dot(plane_.GetNormal()) /
      trajectory.GetLine().GetV0().dot(plane_.GetNormal());

  if (timeOfIntersection < TimeType::zero()) {
    return std::numeric_limits<double>::infinity() * 1_m;
  }

  auto const pointOfIntersection = trajectory.GetLine().GetPosition(timeOfIntersection);
  return (trajectory.GetLine().GetR0() - pointOfIntersection).norm() * 1.0001;
}

void HistoryObservationPlane::fillHistoryHistogram(
    setup::Stack::ParticleType const& muon) {
  double const muon_energy = muon.GetEnergy() / 1_GeV;

  int genctr{0};
  Event const* event = muon.GetEvent().get();
  while (event) {
    auto const projectile = stack_.cfirst() + event->projectileIndex();
    if (event->eventType() == EventType::Interaction) {
      genctr++;
      double const projEnergy = projectile.GetEnergy() / 1_GeV;
      int const pdg = static_cast<int>(particles::GetPDG(projectile.GetPID()));

      histogram_(muon_energy, projEnergy, pdg);
    }
    event = event->parentEvent().get(); // projectile.GetEvent().get();
  }
}
