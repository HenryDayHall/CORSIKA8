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

namespace corsika::observation_plane {

  /**
   * The ObservationPlane writes PDG codes, energies, and distances of particles to the
   * central point of the plane into its output file. The particles are considered
   * "absorbed" afterwards.
   */
  class ObservationPlane : public corsika::ContinuousProcess<ObservationPlane> {

  public:
    ObservationPlane(corsika::Plane const&, std::string const&, bool = true);
    void Init() {}

    corsika::ProcessReturn doContinuous(
        corsika::setup::Stack::ParticleType const& vParticle,
        corsika::setup::Trajectory const& vTrajectory);

    LengthType MaxStepLength(corsika::setup::Stack::ParticleType const&,
                             corsika::setup::Trajectory const& vTrajectory);

  private:
    corsika::Plane const plane_;
    std::ofstream outputStream_;
    bool const deleteOnHit_;
  };
} // namespace corsika::observation_plane

#include <corsika/detail/modules/ObservationPlane.inl>
