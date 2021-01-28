/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <fstream>
#include <string>

namespace corsika {

  /**
   * The ObservationPlane writes PDG codes, energies, and distances of particles to the
   * central point of the plane into its output file. The particles are considered
   * "absorbed" afterwards.
   */
  class ObservationPlane : public ContinuousProcess<ObservationPlane> {

  public:
    ObservationPlane(Plane const&, DirectionVector const&, std::string const&,
                     bool = true);

    ProcessReturn doContinuous(corsika::setup::Stack::particle_type& vParticle,
                               corsika::setup::Trajectory& vTrajectory,
                               bool const stepLimit);

    LengthType getMaxStepLength(corsika::setup::Stack::particle_type const&,
                                corsika::setup::Trajectory const& vTrajectory);

    void showResults() const;
    void reset();
    HEPEnergyType getEnergyGround() const { return energy_ground_; }

  private:
    Plane const plane_;
    std::ofstream outputStream_;
    bool const deleteOnHit_;
    HEPEnergyType energy_ground_;
    unsigned int count_ground_;
    DirectionVector const xAxis_;
    DirectionVector const yAxis_;
  };
} // namespace corsika

#include <corsika/detail/modules/ObservationPlane.inl>
