/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <interface/CorsikaInterface.h>

#include <corsika/coast/COASTProcess.h>
#include <corsika/coast/COASTStack.h>
#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/geometry/Line.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>

#include <crs/CInteraction.h>
#include <crs/CParticle.h>
#include <crs/CorsikaTypes.h>
#include <crs/TSubBlock.h>

#include <iostream>
#include <sstream>

using namespace std;
using namespace corsika;
using namespace corsika::units::si;

corsika::coast::COASTProcess gCorsikaProcess;

/*
  Data is one CORSIKA data-block constining of 21 SubBlocks.
  A SubBlock can be:
  - thinned mode:     39 (Particles) * 8 (ENTRIES) * 4 (BYTES)
  - not-thinned mode: 39 (Particles) * 7 (ENTRIES) * 4 (BYTES)
*/
extern "C" void wrida_([[maybe_unused]] const CREAL* Data) {
  // crs::CParticleFortranPtr p;
  // const bool isF = prminfo_(p);
}

extern "C" void inida_([[maybe_unused]] const char* filename,
                       [[maybe_unused]] const int& thinning,
                       [[maybe_unused]] const int& /*curved*/,
                       [[maybe_unused]] const int& /*slant*/,
                       [[maybe_unused]] const int& /*stackinput*/,
                       [[maybe_unused]] const int& /*preshower*/,
                       [[maybe_unused]] int str_length) {
  gCorsikaProcess.Init();
}

extern "C" void cloda_() {
  // crs::CParticleFortranPtr pptr;
  // const bool isF = prminfo_(pptr);
  // gCorsikaProcess.Close();
}

void interaction_([[maybe_unused]] const crs::CInteraction& interaction) {
  /*
    all interactions in the shower are available in this function !
    the information availabel in the CInteraction class are:
    double x;
    double y;
    double z;
    double etot;      // lab energy
    double sigma;     // cross-section of process
    double kela;      // elasticity
    int    projId;    // projectile
    int    targetId;  // target
    double time;
  */
}

extern "C" void track_([[maybe_unused]] const crs::CParticle& pre,
                       [[maybe_unused]] const crs::CParticle& post) {
  /*
    all particles in the shower are available in this function !
    The pre and post objecte are the two endpoints for one single track
    in the shower, where the information available in CParticle is:
    double x;
    double y;
    double z;
    double depth;
    double time;
    double energy;
    double weight;
    int    particleId;
    int    hadronicGeneration;
  */
  coast::COASTStack stack(&pre, &post);
  const auto particle = stack.GetNextParticle();
  const geometry::CoordinateSystem& rootCS =
      geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
  geometry::Line const line(particle.GetPosition(rootCS), particle.GetVelocity(rootCS));
  const TimeType time = particle.GetTimeInterval();
  const geometry::Trajectory<geometry::Line> track(line, time);
  gCorsikaProcess.DoContinuous(particle, track, stack);
}

extern "C" void tabularizedatmosphere_([[maybe_unused]] const int& nPoints,
                                       [[maybe_unused]] const double* height,
                                       [[maybe_unused]] const double* refractiveIndex) {
  // for special use only but should be defined because it is delcared in CORSIKA.F
}
