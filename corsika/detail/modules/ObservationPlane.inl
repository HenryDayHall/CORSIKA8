/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

namespace corsika {

  template <typename TOutput>
  ObservationPlane<TOutput>::ObservationPlane(Plane const& obsPlane,
                                              DirectionVector const& x_axis,
                                              TOutput& output, bool deleteOnHit)
      : plane_(obsPlane)
      , output_(output)
      , deleteOnHit_(deleteOnHit)
      , energy_ground_(0_GeV)
      , count_ground_(0)
      , xAxis_(x_axis.normalized())
      , yAxis_(obsPlane.getNormal().cross(xAxis_)) {}

  template <typename TTracking, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ObservationPlane<TTracking, TOutput>::doContinuous(
      TParticle& particle, TTrajectory& step, bool const stepLimit) {
    /*
       The current step did not yet reach the ObservationPlane, do nothing now and wait:
     */
    if (!stepLimit) {
      // @todo this is actually needed to fix small instabilities of the leap-frog
      // tracking: Note, this is NOT a general solution and should be clearly revised with
      // a more robust tracking. #ifdef DEBUG
      if (deleteOnHit_) {
        // since this is basically a bug, it cannot be tested LCOV_EXCL_START
        LengthType const check =
            (particle.getPosition() - plane_.getCenter()).dot(plane_.getNormal());
        if (check < 0_m) {
          CORSIKA_LOG_WARN("PARTICLE AVOIDED OBSERVATIONPLANE {}", check);
          CORSIKA_LOG_WARN("Temporary fix: write and remove particle.");
        } else
          return ProcessReturn::Ok;
        // LCOV_EXCL_STOP
      } else
        // #endif
        return ProcessReturn::Ok;
    }

    HEPEnergyType const energy = particle.getEnergy();
    Point const pointOfIntersection = step.getPosition(1);
    Vector const displacement = pointOfIntersection - plane_.getCenter();

    double const weight = 1.0;
    Code const pid = particle.getPID();
    if (pid == Code::Nucleus) {
      // add our particles to the output file stream
      output_.write(particle.getNuclearA(), particle.getNuclearZ(), energy,
                    displacement.dot(xAxis_), displacement.dot(yAxis_), 0_m, weight);
    } else {
      // add our particles to the output file stream
      output_.write(particle.getPID(), energy, displacement.dot(xAxis_),
                    displacement.dot(yAxis_), 0_m, weight);
    }

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
      TParticle const& particle, TTrajectory const& trajectory) {

    CORSIKA_LOG_TRACE("getMaxStepLength, particle={}, pos={}, dir={}, plane={}",
                      particle.asString(), particle.getPosition(),
                      particle.getDirection(), plane_.asString());

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
    CORSIKA_LOG_TRACE("ObservationPlane: getMaxStepLength dist={} m, pos={}",
                      trajectory.getLength(fractionOfIntersection) / 1_m,
                      trajectory.getPosition(fractionOfIntersection));
    return trajectory.getLength(fractionOfIntersection);
  }

  template <typename TTracking, typename TOutput>
  inline void ObservationPlane<TTracking, TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        "\n ******************************\n"
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
