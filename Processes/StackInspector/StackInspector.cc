
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/geometry/RootCoordinateSystem.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/logging/Logger.h>

#include <corsika/setup/SetupTrajectory.h>

#include <iostream>
#include <limits>
using namespace std;

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;

template <typename Stack>
StackInspector<Stack>::StackInspector(const bool aReport)
    : fReport(aReport)
    , fCountStep(0) {}

template <typename Stack>
StackInspector<Stack>::~StackInspector() {}

template <typename Stack>
process::EProcessReturn StackInspector<Stack>::DoContinuous(Particle&, setup::Trajectory&,
                                                            Stack& s) const {
  if (!fReport) return EProcessReturn::eOk;
  [[maybe_unused]] int i = 0;
  EnergyType Etot = 0_GeV;

  for (auto& iterP : s) {
    EnergyType E = iterP.GetEnergy();
    Etot += E;
    geometry::CoordinateSystem& rootCS =
        geometry::RootCoordinateSystem::GetInstance().GetRootCS(); // for printout
    auto pos = iterP.GetPosition().GetCoordinates(rootCS);
    cout << "StackInspector: i=" << setw(5) << fixed << (i++) << ", id=" << setw(30)
         << iterP.GetPID() << " E=" << setw(15) << scientific << (E / 1_GeV) << " GeV, "
         << " pos=" << pos << endl;
  }
  fCountStep++;
  cout << "StackInspector: nStep=" << fCountStep << " stackSize=" << s.GetSize()
       << " Estack=" << Etot / 1_GeV << " GeV" << endl;
  return EProcessReturn::eOk;
}

template <typename Stack>
double StackInspector<Stack>::MaxStepLength(Particle&, setup::Trajectory&) const {
  return std::numeric_limits<double>::infinity();
}

template <typename Stack>
void StackInspector<Stack>::Init() {
  fCountStep = 0;
}

#include <corsika/setup/SetupStack.h>

template class process::stack_inspector::StackInspector<setup::Stack>;
