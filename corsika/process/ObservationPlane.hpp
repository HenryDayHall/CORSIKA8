/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/sequence/ContinuousProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <fstream>

#include "corsika/setup/SetupStack.hpp"
#include "corsika/setup/SetupTrajectory.hpp"

namespace corsika::observation_plane {

  /**
   * The ObservationPlane writes PDG codes, energies, and distances of particles to the
   * central point of the plane into its output file. The particles are considered
   * "absorbed" afterwards.
   */
  class ObservationPlane : public corsika::ContinuousProcess<ObservationPlane> {

  public:
    ObservationPlane(geometry::Plane const&,
                     geometry::Vector<units::si::dimensionless_d> const&,
                     std::string const&, bool = true);

    corsika::EProcessReturn DoContinuous(corsika::Stack::ParticleType const& vParticle,
                                         corsika::Trajectory const& vTrajectory);

    corsika::units::si::LengthType MaxStepLength(corsika::Stack::ParticleType const&,
                                                 corsika::Trajectory const& vTrajectory);

    void ShowResults() const;
    void Reset();
    corsika::units::si::HEPEnergyType GetEnergyGround() const { return energy_ground_; }

  private:
    geometry::Plane const plane_;
    std::ofstream outputStream_;
    bool const deleteOnHit_;

    units::si::HEPEnergyType energy_ground_ = 0 * units::si::electronvolt;
    unsigned int count_ground_ = 0;
    geometry::Vector<units::si::dimensionless_d> const xAxis_, yAxis_;
  };
} // namespace corsika::observation_plane
