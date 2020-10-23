/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Plane.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <fstream>

namespace corsika::process::observation_plane {

  /**
   * The ObservationPlane writes PDG codes, energies, and distances of particles to the
   * central point of the plane into its output file. The particles are considered
   * "absorbed" afterwards.
   */
  class ObservationPlane : public corsika::process::ContinuousProcess<ObservationPlane> {

  public:
    ObservationPlane(geometry::Plane const&, std::string const&, bool = true);

    corsika::process::EProcessReturn DoContinuous(
        corsika::setup::Stack::ParticleType& vParticle,
        corsika::setup::Trajectory const& vTrajectory);

    corsika::units::si::LengthType MaxStepLength(
        corsika::setup::Stack::ParticleType const&,
        corsika::setup::Trajectory const& vTrajectory);

    void ShowResults() const;
    void Reset();
    corsika::units::si::HEPEnergyType GetEnergyGround() const { return energy_ground_; }

  private:
    geometry::Plane const plane_;
    std::ofstream outputStream_;
    bool const deleteOnHit_;

    units::si::HEPEnergyType energy_ground_ = 0 * units::si::electronvolt;
    unsigned int count_ground_ = 0;
  };
} // namespace corsika::process::observation_plane
