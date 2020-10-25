/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Plane.h>
#include <corsika/output/ObservationPlaneWriterParquet.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/logging/Logging.h>

namespace corsika::process::observation_plane {

  /**
   * The ObservationPlane writes PDG codes, energies, and distances of particles to the
   * central point of the plane into its output file. The particles are considered
   * "absorbed" afterwards.
   */
  template <typename TOutputWriter = output::ObservationPlaneWriterParquet>
  class ObservationPlane final
      : public process::ContinuousProcess<ObservationPlane<TOutputWriter>>,
        public TOutputWriter {

  public:
    template <typename... TArgs>
    ObservationPlane(geometry::Plane const& plane, bool const deleteOnHit,
                     TArgs&&... args)
        : TOutputWriter(args...)
        , plane_(plane)
        , deleteOnHit_(deleteOnHit)
        , energy_ground_(units::si::HEPEnergyType::zero())
        , count_ground_(0) {}

    process::EProcessReturn DoContinuous(setup::Stack::ParticleType& particle,
                                         setup::Trajectory const& trajectory) {

      using namespace units::si;
      TimeType const timeOfIntersection =
          (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
          trajectory.GetV0().dot(plane_.GetNormal());

      if (timeOfIntersection < TimeType::zero()) { return process::EProcessReturn::eOk; }

      if (plane_.IsAbove(trajectory.GetR0()) ==
          plane_.IsAbove(trajectory.GetPosition(1))) {
        return process::EProcessReturn::eOk;
      }

      const auto energy = particle.GetEnergy();

      // write the data to the output
      this->Write(particle.GetPID(), energy,
                  (trajectory.GetPosition(1) - plane_.GetCenter()).norm());

      if (deleteOnHit_) {
        count_ground_++;
        energy_ground_ += energy;
        particle.Delete();
        return process::EProcessReturn::eParticleAbsorbed;
      } else {
        return process::EProcessReturn::eOk;
      }
    }

    units::si::LengthType MaxStepLength(setup::Stack::ParticleType& particle,
                                        setup::Trajectory const& trajectory) {

      using namespace units::si;
      TimeType const timeOfIntersection =
          (plane_.GetCenter() - trajectory.GetR0()).dot(plane_.GetNormal()) /
          trajectory.GetV0().dot(plane_.GetNormal());

      if (timeOfIntersection < TimeType::zero()) {
        return std::numeric_limits<double>::infinity() * 1_m;
      }

      auto const pointOfIntersection = trajectory.GetPosition(timeOfIntersection);
      auto dist = (trajectory.GetR0() - pointOfIntersection).norm() * 1.0001;
      C8LOG_TRACE("ObservationPlane::MaxStepLength l={} m", dist / 1_m);
      return dist;
    }

    YAML::Node GetConfig() const {
      using namespace units::si;

      // construct the top-level node
      YAML::Node node;

      // basic info
      node["name"] = this->name_;
      node["type"] = "ObservationPlane";

      // the center of the plane
      auto const center{plane_.GetCenter()};
      node["plane"]["center"].push_back(center.GetX() / 1_m);
      node["plane"]["center"].push_back(center.GetY() / 1_m);
      node["plane"]["center"].push_back(center.GetZ() / 1_m);
      node["plane"]["center.units"] = "m";

      // the normal vector
      auto const normal{plane_.GetNormal().GetComponents()};
      node["plane"]["normal"].push_back(normal.GetX().magnitude());
      node["plane"]["normal"].push_back(normal.GetY().magnitude());
      node["plane"]["normal"].push_back(normal.GetZ().magnitude());

      node["delete_on_hit"] = deleteOnHit_;

      return node;
    }

    void ShowResults() const {
      using namespace units::si;
      C8LOG_INFO(
          " ******************************\n"
          " ObservationPlane: \n"
          " energy in ground (GeV)     :  {}\n"
          " no. of particles in ground :  {}\n"
          " ******************************",
          energy_ground_ / 1_GeV, count_ground_);
    }

    void Reset() {
      using namespace units::si;
      energy_ground_ = 0_GeV;
      count_ground_ = 0;
    }
    corsika::units::si::HEPEnergyType GetEnergyGround() const { return energy_ground_; }

  private:
    geometry::Plane const plane_;
    bool const deleteOnHit_;

    units::si::HEPEnergyType energy_ground_ = 0 * units::si::electronvolt;
    unsigned int count_ground_ = 0;
  };
} // namespace corsika::process::observation_plane
