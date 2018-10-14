
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

#include <iostream>
using namespace std;

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;

template <typename Stack, typename Trajectory>
StackInspector<Stack, Trajectory>::StackInspector(const bool aReport)
    : fReport(aReport) {}

template <typename Stack, typename Trajectory>
StackInspector<Stack, Trajectory>::~StackInspector() {}

template <typename Stack, typename Trajectory>
process::EProcessReturn StackInspector<Stack, Trajectory>::DoContinuous(Particle&,
                                                                        Trajectory&,
                                                                        Stack& s) const {

  // using namespace corsika::particles::io;

  static int countStep = 0;
  if (!fReport) return EProcessReturn::eOk;
  // std::cout << "generation  " << countStep << std::endl;
  [[maybe_unused]] int i = 0;
  EnergyType Etot = 0_GeV;
  for (auto& iterP : s) {
    EnergyType E = iterP.GetEnergy();
    Etot += E;
    // std::cout << " particle data: " << i++ << ", id=" << iterP << " | " << std::endl;
  }
  countStep++;
  // cout << "#=" << countStep << " " << s.GetSize() << " " << Etot/1_GeV << endl;
  cout << countStep << " " << s.GetSize() << " " << Etot / 1_GeV << " " << endl;
  return EProcessReturn::eOk;
}

template <typename Stack, typename Trajectory>
double StackInspector<Stack, Trajectory>::MinStepLength(Particle&) const {
  return 0;
}

template <typename Stack, typename Trajectory>
void StackInspector<Stack, Trajectory>::DoDiscrete(Particle&, Stack&) const {}

template <typename Stack, typename Trajectory>
void StackInspector<Stack, Trajectory>::Init() {}

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

template class corsika::process::stack_inspector::StackInspector<setup::Stack,
                                                                 setup::Trajectory>;
