/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

namespace corsika {

  template <typename TTracking, typename TVolume, typename TOutput>
  ObservationVolume<TTracking, TVolume, TOutput>::ObservationVolume(TVolume vol)
      : vol_(vol)
      , energy_(0_GeV)
      , count_(0) {}

  template <typename TTracking, typename TVolume, typename TOutput>
  template <typename TParticle>
  inline ProcessReturn ObservationVolume<TTracking, TVolume, TOutput>::doContinuous(
      Step<TParticle>& step, bool const stepLimit) {
    /*
       The current step did not yet reach the ObservationVolume, do nothing now and
       wait:
     */
    if (!stepLimit) { return ProcessReturn::Ok; }

    HEPEnergyType const energy = step.getEkinPost();
    Point const pointOfIntersection = step.getPositionPost();
    DirectionVector const dirction = step.getDirectionPost();

    // add particles to the output file stream
    auto cs = vol_.getCoordinateSystem();
    this->write(step.getParticlePre().getPID(), energy, pointOfIntersection.getX(cs),
                pointOfIntersection.getY(cs), pointOfIntersection.getZ(cs),
                dirction.getX(cs), dirction.getY(cs), dirction.getZ(cs),
                step.getTimePost());

    // always absorb particles
    count_++;
    energy_ += energy;
    return ProcessReturn::ParticleAbsorbed;
  }

  template <typename TTracking, typename TVolume, typename TOutput>
  template <typename TParticle, typename TTrajectory>
  inline LengthType ObservationVolume<TTracking, TVolume, TOutput>::getMaxStepLength(
      TParticle const& particle, TTrajectory const& trajectory) {

    CORSIKA_LOG_TRACE("getMaxStepLength, particle={}, pos={}, dir={}, Box={}",
                      particle.asString(), particle.getPosition(),
                      particle.getDirection(), vol_.asString());

    auto const intersection = TTracking::intersect(particle, vol_);

    TimeType const timeOfEntry = intersection.getEntry();
    TimeType const timeOfExit = intersection.getExit();
    CORSIKA_LOG_TRACE("timeOfEntry={}, timeOfExit={}", timeOfEntry, timeOfExit);
    if (timeOfEntry < TimeType::zero()) {
      if (timeOfExit < TimeType::zero()) {
        // opposite direction
        return std::numeric_limits<double>::infinity() * 1_m;
      } else {
        // inside box: not zero but 1 pm, to allow short lived particles to decay
        return 1e-12 * 1_m;
      }
    }
    if (timeOfEntry > trajectory.getDuration()) {
      // can not reach
      return std::numeric_limits<double>::infinity() * 1_m;
    }
    double const fractionOfIntersection = timeOfEntry / trajectory.getDuration();
    CORSIKA_LOG_TRACE("ObservationVolume: getMaxStepLength dist={} m, pos={}",
                      trajectory.getLength(fractionOfIntersection) / 1_m,
                      trajectory.getPosition(fractionOfIntersection));
    return trajectory.getLength(fractionOfIntersection);
  }

  template <typename TTracking, typename TVolume, typename TOutput>
  inline void ObservationVolume<TTracking, TVolume, TOutput>::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " ObservationVolume: \n"
        " energy at Box (GeV)     :  {}\n"
        " no. of particles at Box :  {}\n"
        " ******************************",
        energy_ / 1_GeV, count_);
  }

  template <typename TTracking, typename TVolume, typename TOutput>
  inline YAML::Node ObservationVolume<TTracking, TVolume, TOutput>::getConfig() const {
    using namespace units::si;
    auto cs = vol_.getCoordinateSystem();
    auto center = vol_.getCenter();

    // construct the top-level node
    YAML::Node node;

    // basic info
    // TODO: At present, we are using the type ObservationPlane to
    // share the same python reader package (io/outputs/observation_plane.py)
    // though they have different data columns
    node["type"] = "ObservationPlane";
    node["units"] = "m"; // add default units for values

    // save each component in its native coordinate system
    auto const root_cs = get_root_CoordinateSystem();
    node["center"].push_back(center.getX(root_cs) / 1_m);
    node["center"].push_back(center.getY(root_cs) / 1_m);
    node["center"].push_back(center.getZ(root_cs) / 1_m);

    // the x-axis vector
    DirectionVector const x_axis = DirectionVector{cs, {1, 0, 0}};
    node["x-axis"].push_back(x_axis.getX(root_cs).magnitude());
    node["x-axis"].push_back(x_axis.getY(root_cs).magnitude());
    node["x-axis"].push_back(x_axis.getZ(root_cs).magnitude());

    // the y-axis vector
    DirectionVector const y_axis = DirectionVector{cs, {0, 1, 0}};
    node["y-axis"].push_back(y_axis.getX(root_cs).magnitude());
    node["y-axis"].push_back(y_axis.getY(root_cs).magnitude());
    node["y-axis"].push_back(y_axis.getZ(root_cs).magnitude());

    // the x-axis vector
    DirectionVector const z_axis = DirectionVector{cs, {0, 0, 1}};
    node["z-axis"].push_back(z_axis.getX(root_cs).magnitude());
    node["z-axis"].push_back(z_axis.getY(root_cs).magnitude());
    node["z-axis"].push_back(z_axis.getZ(root_cs).magnitude());

    return node;
  }

  template <typename TTracking, typename TVolume, typename TOutput>
  inline void ObservationVolume<TTracking, TVolume, TOutput>::reset() {
    energy_ = 0_GeV;
    count_ = 0;
  }

} // namespace corsika
