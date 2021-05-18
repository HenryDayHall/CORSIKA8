/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

namespace corsika {

  template <typename TTracking, typename TOutput>
  ObservationPlane<TTracking, TOutput>::ObservationPlane(Plane const& obsPlane,
                                              DirectionVector const& x_axis,
                                              bool deleteOnHit)
      : plane_(obsPlane)
      , deleteOnHit_(deleteOnHit)
      , energy_ground_(0_GeV)
      , count_ground_(0)
      , xAxis_(x_axis.normalized())
      , yAxis_(obsPlane.getNormal().cross(xAxis_)) {}

  template <typename TTracking, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ObservationPlane<TTracking, TOutput>::doContinuous(
      TParticle& particle, TTrajectory&,
      bool const stepLimit) {
    /*
       The current step did not yet reach the ObservationPlane, do nothing now and wait:
     */
    if (!stepLimit) {
#ifdef DEBUG
      if (deleteOnHit_) {
        LengthType const check =
            (particle.getPosition() - plane_.getCenter()).dot(plane_.getNormal());
        if (check < 0_m) {
          CORSIKA_LOG_DEBUG("PARTICLE AVOIDED OBSERVATIONPLANE {}", check);
        }
      }
#endif
      return ProcessReturn::Ok;
    }

    HEPEnergyType const energy = particle.getEnergy();
    Point const pointOfIntersection = particle.getPosition();
    Vector const displacement = pointOfIntersection - plane_.getCenter();

    // add our particles to the output file stream
    this->write(particle.getPID(), energy, displacement.dot(xAxis_),
                displacement.dot(yAxis_));

    CORSIKA_LOG_TRACE("Particle detected absorbed={}", deleteOnHit_);

    if (deleteOnHit_) {
      count_ground_++;
      energy_ground_ += energy;
      return ProcessReturn::ParticleAbsorbed;
    } else {
      return ProcessReturn::Ok;
    }
  }

  template <typename TTracking, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline LengthType ObservationPlane<TTracking, TOutput>::getMaxStepLength(
      TParticle const& particle,
      TTrajectory const& trajectory) {

    CORSIKA_LOG_TRACE("particle={}, pos={}, dir={}, plane={}", particle.asString(),
                      particle.getPosition(), particle.getDirection(), plane_.asString());

    auto const intersection = TTracking::intersect(particle, plane_);

    TimeType const timeOfIntersection = intersection.getEntry();
    CORSIKA_LOG_TRACE("timeOfIntersection={}", timeOfIntersection);
    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }
    if (timeOfIntersection > trajectory.getDuration()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }
    double const fractionOfIntersection = timeOfIntersection / trajectory.getDuration();
    auto const pointOfIntersection = trajectory.getPosition(fractionOfIntersection);
    auto dist = (trajectory.getPosition(0) - pointOfIntersection).getNorm();
    CORSIKA_LOG_TRACE("ObservationPlane: getMaxStepLength l={} m", dist / 1_m);
    return dist;
  }

  template <typename TTracking, typename TOutput>
  inline void ObservationPlane<TTracking, TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " ObservationPlane: \n"
        " energy an ground (GeV)     :  {}\n"
        " no. of particles at ground :  {}\n"
        " ******************************",
        energy_ground_ / 1_GeV, count_ground_);
  }

  template <typename TTracking, typename TOutput>
  inline YAML::Node ObservationPlane<TTracking, TOutput>::getConfig() const {
    using namespace units::si;

    // construct the top-level node
    YAML::Node node;

    // basic info
    node["type"] = "ObservationPlane";
    node["units"] = "m"; // add default units for values

    // the center of the plane
    auto const center{plane_.getCenter()};

    // save each component in its native coordinate system
    auto const center_coords{center.getCoordinates(center.getCoordinateSystem())};
    node["plane"]["center"].push_back(center_coords.getX() / 1_m);
    node["plane"]["center"].push_back(center_coords.getY() / 1_m);
    node["plane"]["center"].push_back(center_coords.getZ() / 1_m);

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

  template <typename TTracking, typename TOutput>
  inline void ObservationPlane<TTracking, TOutput>::reset() {
    energy_ground_ = 0_GeV;
    count_ground_ = 0;
  }

} // namespace corsika
