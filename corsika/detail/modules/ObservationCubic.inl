/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

namespace corsika {

  template <typename TTracking, typename TOutput>
  ObservationCubic<TTracking, TOutput>::ObservationCubic(
      Point const& center, CoordinateSystemPtr cs, LengthType const x, LengthType const y,
      LengthType const z, bool deleteOnHit)
      : Cubic(center, cs, x, y, z)
      , deleteOnHit_(deleteOnHit)
      , energy_(0_GeV)
      , count_(0) {}

  template <typename TTracking, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn ObservationCubic<TTracking, TOutput>::doContinuous(
      TParticle& particle, TTrajectory& step, bool const stepLimit) {
    /*
       The current step did not yet reach the ObservationCubic, do nothing now and
       wait:
     */
    if (!stepLimit) {
      // @todo this is actually needed to fix small instabilities of the leap-frog
      // tracking: Note, this is NOT a general solution and should be clearly
      // revised with a more robust tracking. #ifdef DEBUG
      if (deleteOnHit_) {
        // since this is basically a bug, it cannot be tested LCOV_EXCL_START
        LengthType const check = 1_m; // TODO, do I need to check?
        if (check < 0_m) {
          CORSIKA_LOG_WARN("PARTICLE AVOIDED ObservationCubic {}", check);
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
    DirectionVector const dirction = particle.getDirection();

    // add our particles to the output file stream
    this->write(particle.getPID(), energy, pointOfIntersection.getX(cs_),
                pointOfIntersection.getY(cs_), pointOfIntersection.getZ(cs_),
                dirction.getX(cs_), dirction.getY(cs_), dirction.getZ(cs_),
                particle.getTime());

    CORSIKA_LOG_TRACE("Particle detected absorbed={}", deleteOnHit_);

    if (deleteOnHit_) {
      count_++;
      energy_ += energy;
      return ProcessReturn::ParticleAbsorbed;
    } else {
      return ProcessReturn::Ok;
    }
  }

  template <typename TTracking, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline LengthType ObservationCubic<TTracking, TOutput>::getMaxStepLength(
      TParticle const& particle, TTrajectory const& trajectory) {

    CORSIKA_LOG_TRACE("getMaxStepLength, particle={}, pos={}, dir={}, cubic={}",
                      particle.asString(), particle.getPosition(),
                      particle.getDirection(), asString());

    auto const intersection =
        TTracking::intersect(particle, static_cast<Cubic const>(*this));

    TimeType const timeOfIntersection = intersection.getEntry();
    CORSIKA_LOG_TRACE("timeOfIntersection={}", timeOfIntersection);
    if (timeOfIntersection < TimeType::zero()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }
    if (timeOfIntersection > trajectory.getDuration()) {
      return std::numeric_limits<double>::infinity() * 1_m;
    }
    double const fractionOfIntersection = timeOfIntersection / trajectory.getDuration();
    CORSIKA_LOG_TRACE("ObservationCubic: getMaxStepLength dist={} m, pos={}",
                      trajectory.getLength(fractionOfIntersection) / 1_m,
                      trajectory.getPosition(fractionOfIntersection));
    return trajectory.getLength(fractionOfIntersection);
  }

  template <typename TTracking, typename TOutput>
  inline void ObservationCubic<TTracking, TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " ObservationCubic: \n"
        " energy at cubic (GeV)     :  {}\n"
        " no. of particles at cubic :  {}\n"
        " ******************************",
        energy_ / 1_GeV, count_);
  }

  template <typename TTracking, typename TOutput>
  inline YAML::Node ObservationCubic<TTracking, TOutput>::getConfig() const {
    using namespace units::si;

    // construct the top-level node
    YAML::Node node;

    // basic info
    node["type"] = "ObservationCubic";
    node["units"] = "m"; // add default units for values

    // save each component in its native coordinate system
    auto const root_cs = get_root_CoordinateSystem();
    node["center"].push_back(center_.getX(root_cs) / 1_m);
    node["center"].push_back(center_.getY(root_cs) / 1_m);
    node["center"].push_back(center_.getZ(root_cs) / 1_m);

    // the x-axis vector
    DirectionVector const x_axis = DirectionVector{cs_, {1, 0, 0}};
    node["x-axis"].push_back(x_axis.getX(root_cs).magnitude());
    node["x-axis"].push_back(x_axis.getY(root_cs).magnitude());
    node["x-axis"].push_back(x_axis.getZ(root_cs).magnitude());

    // the y-axis vector
    DirectionVector const y_axis = DirectionVector{cs_, {0, 1, 0}};
    node["y-axis"].push_back(y_axis.getX(root_cs).magnitude());
    node["y-axis"].push_back(y_axis.getY(root_cs).magnitude());
    node["y-axis"].push_back(y_axis.getZ(root_cs).magnitude());

    // the x-axis vector
    DirectionVector const z_axis = DirectionVector{cs_, {0, 0, 1}};
    node["z-axis"].push_back(z_axis.getX(root_cs).magnitude());
    node["z-axis"].push_back(z_axis.getY(root_cs).magnitude());
    node["z-axis"].push_back(z_axis.getZ(root_cs).magnitude());

    node["delete_on_hit"] = deleteOnHit_;

    return node;
  }

  template <typename TTracking, typename TOutput>
  inline void ObservationCubic<TTracking, TOutput>::reset() {
    energy_ = 0_GeV;
    count_ = 0;
  }

} // namespace corsika
