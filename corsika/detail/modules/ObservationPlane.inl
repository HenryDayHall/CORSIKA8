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

  ObservationPlane::ObservationPlane(Plane const& obsPlane, DirectionVector const& x_axis,
                                     std::string const& filename, bool deleteOnHit)
      : plane_(obsPlane)
      , outputStream_(filename)
      , deleteOnHit_(deleteOnHit)
      , energy_ground_(0_GeV)
      , count_ground_(0)
      , xAxis_(x_axis.normalized())
      , yAxis_(obsPlane.getNormal().cross(xAxis_)) {
    outputStream_ << "#PDG code, energy / eV, x distance / m, y distance / m"
                  << std::endl;
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
    auto const displacement = trajectory.getPosition(1) - plane_.getCenter();

    outputStream_ << static_cast<int>(get_PDG(particle.getPID())) << ' ' << energy / 1_eV
                  << ' ' << displacement.dot(xAxis_) / 1_m << ' '
                  << displacement.dot(yAxis_) / 1_m
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
      corsika::setup::Stack::particle_type const& vParticle,
      corsika::setup::Trajectory const& trajectory) {

    int chargeNumber;
    if (is_nucleus(vParticle.getPID())) {
      chargeNumber = vParticle.getNuclearZ();
    } else {
      chargeNumber = get_charge_number(vParticle.getPID());
    }
    auto const* currentLogicalVolumeNode = vParticle.getNode();
    auto magneticfield = currentLogicalVolumeNode->getModelProperties().getMagneticField(
        vParticle.getPosition());
    auto direction = trajectory.getLine().getV0().normalized();

    if (chargeNumber != 0 &&
        abs(plane_.getNormal().dot(trajectory.getLine().getV0().cross(magneticfield))) *
                1_s / 1_m / 1_T >
            1e-6) {
      auto const* currentLogicalVolumeNode = vParticle.getNode();
      auto magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(
              vParticle.getPosition());
      auto k =
          chargeNumber * constants::c * 1_eV / (vParticle.getMomentum().norm() * 1_V);

      if (direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
              (plane_.getNormal().dot(trajectory.getLine().getR0() - plane_.getCenter()) *
               plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k) <
          0) {
        return std::numeric_limits<double>::infinity() * 1_m;
      }

      LengthType MaxStepLength1 =
          (sqrt(direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
                (plane_.getNormal().dot(trajectory.getLine().getR0() -
                                        plane_.getCenter()) *
                 plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
           direction.dot(plane_.getNormal()) / direction.getNorm()) /
          (plane_.getNormal().dot(direction.cross(magneticfield)) * k);

      LengthType MaxStepLength2 =
          (-sqrt(direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
                 (plane_.getNormal().dot(trajectory.getLine().getR0() -
                                         plane_.getCenter()) *
                  plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
           direction.dot(plane_.getNormal()) / direction.getNorm()) /
          (plane_.getNormal().dot(direction.cross(magneticfield)) * k);

      if (MaxStepLength1 <= 0_m && MaxStepLength2 <= 0_m) {
        return std::numeric_limits<double>::infinity() * 1_m;
      } else if (MaxStepLength1 <= 0_m || MaxStepLength2 < MaxStepLength1) {
        std::cout << " steplength to obs plane 2: " << MaxStepLength2 << std::endl;
        return MaxStepLength2 *
               (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                   .norm() *
               1.001;
      } else if (MaxStepLength2 <= 0_m || MaxStepLength1 < MaxStepLength2) {
        std::cout << " steplength to obs plane 1: " << MaxStepLength1 << std::endl;
        return MaxStepLength1 *
               (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                   .norm() *
               1.001;
      }
    }

    TimeType const timeOfIntersection =
        (plane_.getCenter() - trajectory.getStartPoint()).dot(plane_.getNormal()) /
        trajectory.getVelocity().dot(plane_.getNormal());

    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }

    auto const pointOfIntersection = trajectory.getPosition(timeOfIntersection);
    auto dist = (trajectory.getStartPoint() - pointOfIntersection).getNorm() * 1.0001;
    CORSIKA_LOG_TRACE("ObservationPlane w/o magnetic field: getMaxStepLength l={} m",
                      dist / 1_m);
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
