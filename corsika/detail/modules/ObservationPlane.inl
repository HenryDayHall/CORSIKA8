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

  template <typename TOutput>
  ObservationPlane<TOutput>::ObservationPlane(Plane const& obsPlane,
                                              DirectionVector const& x_axis,
                                              bool deleteOnHit)

      : plane_(obsPlane)
      , deleteOnHit_(deleteOnHit)
      , energy_ground_(0_GeV)
      , count_ground_(0)
      , xAxis_(x_axis.normalized())
      , yAxis_(obsPlane.getNormal().cross(xAxis_)) {}

  template <typename TOutput>
  ProcessReturn ObservationPlane<TOutput>::doContinuous(
      corsika::setup::Stack::particle_type& particle,
      corsika::setup::Trajectory& trajectory) {
    TimeType const timeOfIntersection =
        (plane_.getCenter() - trajectory.getPosition(0)).dot(plane_.getNormal()) /
        trajectory.getVelocity(0).dot(plane_.getNormal());

    if (timeOfIntersection < TimeType::zero()) { return ProcessReturn::Ok; }

    if (plane_.isAbove(trajectory.getPosition(0)) ==
        plane_.isAbove(trajectory.getPosition(1))) {
      return ProcessReturn::Ok;
    }

    const auto energy = particle.getEnergy();
    auto const displacement = trajectory.getPosition(1) - plane_.getCenter();

    // add our particles to the output file stream
    this->write(particle.getPID(), energy,
                displacement.dot(xAxis_),
                displacement.dot(yAxis_));

    if (deleteOnHit_) {
      count_ground_++;
      energy_ground_ += energy;
      particle.erase();
      return ProcessReturn::ParticleAbsorbed;
    } else {
      return ProcessReturn::Ok;
    }
  }

  template <typename TOutput>
  LengthType ObservationPlane<TOutput>::getMaxStepLength(
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
    auto direction = trajectory.getVelocity(0).normalized();

    if (chargeNumber != 0 &&
        abs(plane_.getNormal().dot(
            trajectory.getLine().getVelocity().cross(magneticfield))) *
                1_s / 1_m / 1_T >
            1e-6) {
      auto const* currentLogicalVolumeNode = vParticle.getNode();
      auto magneticfield =
          currentLogicalVolumeNode->getModelProperties().getMagneticField(
              vParticle.getPosition());
      auto k =
          chargeNumber * constants::c * 1_eV / (vParticle.getMomentum().getNorm() * 1_V);

      if (direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
              (plane_.getNormal().dot(trajectory.getPosition(0) - plane_.getCenter()) *
               plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k) <
          0) {
        return std::numeric_limits<double>::infinity() * 1_m;
      }

      LengthType MaxStepLength1 =
          (sqrt(direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
                (plane_.getNormal().dot(trajectory.getPosition(0) - plane_.getCenter()) *
                 plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
           direction.dot(plane_.getNormal()) / direction.getNorm()) /
          (plane_.getNormal().dot(direction.cross(magneticfield)) * k);

      LengthType MaxStepLength2 =
          (-sqrt(direction.dot(plane_.getNormal()) * direction.dot(plane_.getNormal()) -
                 (plane_.getNormal().dot(trajectory.getPosition(0) - plane_.getCenter()) *
                  plane_.getNormal().dot(direction.cross(magneticfield)) * 2 * k)) -
           direction.dot(plane_.getNormal()) / direction.getNorm()) /
          (plane_.getNormal().dot(direction.cross(magneticfield)) * k);

      if (MaxStepLength1 <= 0_m && MaxStepLength2 <= 0_m) {
        return std::numeric_limits<double>::infinity() * 1_m;
      } else if (MaxStepLength1 <= 0_m || MaxStepLength2 < MaxStepLength1) {
        std::cout << " steplength to obs plane 2: " << MaxStepLength2 << std::endl;
        return MaxStepLength2 *
               (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                   .getNorm() *
               1.001;
      } else if (MaxStepLength2 <= 0_m || MaxStepLength1 < MaxStepLength2) {
        std::cout << " steplength to obs plane 1: " << MaxStepLength1 << std::endl;
        return MaxStepLength1 *
               (direction + direction.cross(magneticfield) * MaxStepLength2 * k / 2)
                   .getNorm() *
               1.001;
      }
    }

    TimeType const timeOfIntersection =
        (plane_.getCenter() - trajectory.getPosition(0)).dot(plane_.getNormal()) /
        trajectory.getVelocity(0).dot(plane_.getNormal());

    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }

    double const fractionOfIntersection = timeOfIntersection / trajectory.getDuration();

    auto const pointOfIntersection = trajectory.getPosition(fractionOfIntersection);
    auto dist = (trajectory.getPosition(0) - pointOfIntersection).getNorm() * 1.0001;
    CORSIKA_LOG_TRACE("ObservationPlane w/o magnetic field: getMaxStepLength l={} m",
                      dist / 1_m);
    return dist;
  }

  template <typename TOutput>
  void ObservationPlane<TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " ObservationPlane: \n"
        " energy an ground (GeV)     :  {}\n"
        " no. of particles at ground :  {}\n"
        " ******************************",
        energy_ground_ / 1_GeV, count_ground_);
  }

  template <typename TOutput>
  YAML::Node ObservationPlane<TOutput>::getConfig() const {
    using namespace units::si;

    // construct the top-level node
    YAML::Node node;

    // basic info
    node["type"] = "ObservationPlane";

    // the center of the plane
    auto const center{plane_.getCenter()};

    // save each component in its native coordinate system
    auto const center_coords{center.getCoordinates(center.getCoordinateSystem())};
    node["plane"]["center"].push_back(center_coords.getX() / 1_m);
    node["plane"]["center"].push_back(center_coords.getY() / 1_m);
    node["plane"]["center"].push_back(center_coords.getZ() / 1_m);
    node["plane"]["center.units"] = "m";

    // the normal vector of the plane
    auto const normal{plane_.getNormal().getComponents()};
    node["plane"]["normal"].push_back(normal.getX().magnitude());
    node["plane"]["normal"].push_back(normal.getY().magnitude());
    node["plane"]["normal"].push_back(normal.getZ().magnitude());

    // the x-axis vector
    auto const xAxis_coords{xAxis_.getComponents(xAxis_.getCoordinateSystem())};
    node["x-axis"].push_back(xAxis_coords.getX().magnitude());
    node["x-axis"].push_back(xAxis_coords.getY().magnitude());
    node["x-axis"].push_back(xAxis_coords.getZ().magnitude());

    // the y-axis vector
    auto const yAxis_coords{yAxis_.getComponents(yAxis_.getCoordinateSystem())};
    node["y-axis"].push_back(yAxis_coords.getX().magnitude());
    node["y-axis"].push_back(yAxis_coords.getY().magnitude());
    node["y-axis"].push_back(yAxis_coords.getZ().magnitude());

    node["delete_on_hit"] = deleteOnHit_;

    return node;
  }

  template <typename TOutput>
  void ObservationPlane<TOutput>::reset() {
    energy_ground_ = 0_GeV;
    count_ground_ = 0;
  }

} // namespace corsika
