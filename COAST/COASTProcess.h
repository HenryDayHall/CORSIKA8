/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_coast_coastprocess_h_
#define _include_corsika_coast_coastprocess_h_

#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/coast/COASTStack.h>

#include <limits>

typedef corsika::coast::COASTStack Stack;
typedef corsika::coast::COASTStack::ParticleType Particle;
typedef corsika::geometry::Trajectory<corsika::geometry::Line> Track;

namespace corsika::coast {

  class COASTProcess : public corsika::process::ContinuousProcess<COASTProcess> {

  public:
    void Init();

    corsika::process::EProcessReturn DoContinuous(const Particle&, const Track&,
                                                  const Stack&);

    corsika::units::si::LengthType MaxStepLength(Particle&, Track&) {
      return corsika::units::si::meter * std::numeric_limits<double>::infinity();
    }

  private:
  };

} // namespace corsika::coast

#endif
