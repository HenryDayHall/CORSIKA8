
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/stack_inspector/StackInspector.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/logging/Logger.h>

#include <corsika/setup/SetupTrajectory.h>

#include <iostream>
using namespace std;

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;

template <typename Stack>
StackInspector<Stack>::StackInspector(const bool aReport)
    : fReport(aReport) {}

template <typename Stack>
StackInspector<Stack>::~StackInspector() {}

template <typename Stack>
process::EProcessReturn StackInspector<Stack>::DoContinuous(
    Particle&, corsika::setup::Trajectory&, Stack& s) const {
  static int countStep = 0;
  if (!fReport) return EProcessReturn::eOk;
  [[maybe_unused]] int i = 0;
  EnergyType Etot = 0_GeV;
  for (auto& iterP : s) {
    EnergyType E = iterP.GetEnergy();
    Etot += E;
    cout << "i=" << setw(5) << fixed << (i++)
	 << ", id=" << setw(30) << iterP.GetPID()
         << " E=" << setw(15) << scientific << (E / 1_GeV) << " GeV, "
         //<< " pos=" << iterP.GetPosition()
         << endl;
  }
  countStep++;
  cout << countStep << " " << s.GetSize() << " " << Etot / 1_GeV << " " << endl;
  return EProcessReturn::eOk;
}

template <typename Stack>
void StackInspector<Stack>::MinStepLength(Particle&,
                                          corsika::setup::Trajectory&) const {
  // return 0;
}

template <typename Stack>
void StackInspector<Stack>::Init() {}

#include <corsika/setup/SetupStack.h>

template class corsika::process::stack_inspector::StackInspector<setup::Stack>;
