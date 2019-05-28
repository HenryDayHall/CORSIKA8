/*
 * (c) Copyright 2019 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Processes_ObservationPlane_h_
#define _Processes_ObservationPlane_h_

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
    ObservationPlane(geometry::Plane const& vObsPlane, std::string const& vFilename);
    void Init() {}

    corsika::process::EProcessReturn DoContinuous(
        corsika::setup::Stack::ParticleType const& vParticle,
        corsika::setup::Trajectory const& vTrajectory);

    corsika::units::si::LengthType MaxStepLength(
        corsika::setup::Stack::ParticleType const&,
        corsika::setup::Trajectory const& vTrajectory);

  private:
    geometry::Plane const fObsPlane;
    std::ofstream fOutputStream;
  };
} // namespace corsika::process::observation_plane

#endif
