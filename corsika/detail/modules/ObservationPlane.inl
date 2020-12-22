/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/ObservationPlane.hpp>

#include <fstream>

namespace corsika {

  ObservationPlane::ObservationPlane(Plane const& obsPlane, std::string const& filename,
                                     bool deleteOnHit)
      : plane_(obsPlane)
      , outputStream_(filename)
      , deleteOnHit_(deleteOnHit)
      , energy_ground_(0_GeV)
      , count_ground_(0) {
    outputStream_ << "#PDG code, energy / eV, distance to center / m" << std::endl;
  }

  ProcessReturn ObservationPlane::doContinuous(
      corsika::setup::Stack::particle_type& particle,
      corsika::setup::Trajectory& trajectory) {
    TimeType const timeOfIntersection =
        (plane_.getCenter() - trajectory.getStartPoint()).dot(plane_.getNormal()) /
        trajectory.getVelocity().dot(plane_.getNormal());

    if (timeOfIntersection < TimeType::zero()) { return ProcessReturn::Ok; }

    if (plane_.isAbove(trajectory.getStartPoint()) ==
        plane_.isAbove(trajectory.getPosition(1))) {
      return ProcessReturn::Ok;
    }

    const auto energy = particle.getEnergy();
    outputStream_ << static_cast<int>(get_PDG(particle.getPID())) << ' ' << energy / 1_eV
                  << ' '
                  << (trajectory.getPosition(1) - plane_.getCenter()).getNorm() / 1_m
                  << std::endl;

    if (deleteOnHit_) {
      count_ground_++;
      energy_ground_ += energy;
      particle.erase();
      return ProcessReturn::ParticleAbsorbed;
    } else {
      return ProcessReturn::Ok;
    }
  }

  LengthType ObservationPlane::getMaxStepLength(
      corsika::setup::Stack::particle_type const&,
      corsika::setup::Trajectory const& trajectory) {

    TimeType const timeOfIntersection =
        (plane_.getCenter() - trajectory.getStartPoint()).dot(plane_.getNormal()) /
        trajectory.getVelocity().dot(plane_.getNormal());

    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }

    auto const pointOfIntersection = trajectory.getPosition(timeOfIntersection);
    auto dist = (trajectory.getStartPoint() - pointOfIntersection).getNorm() * 1.0001;
    CORSIKA_LOG_TRACE("ObservationPlane::getMaxStepLength l={} m", dist / 1_m);
    return dist;
  }

  void ObservationPlane::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " ObservationPlane: \n"
        " energy in ground (GeV)     :  {}\n"
        " no. of particles in ground :  {}\n"
        " ******************************",
        energy_ground_ / 1_GeV, count_ground_);
  }

  void ObservationPlane::reset() {
    energy_ground_ = 0_GeV;
    count_ground_ = 0;
  }

} // namespace corsika
