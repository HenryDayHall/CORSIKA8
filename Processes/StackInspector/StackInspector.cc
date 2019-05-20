/*
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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
using namespace std;

using namespace corsika;
using namespace corsika::particles;
using namespace corsika::units::si;
using namespace corsika::process::stack_inspector;

template <typename TStack>
StackInspector<TStack>::StackInspector(const int vNStep, const bool vReportStack,
                                       const HEPEnergyType vE0)
    : StackProcess<StackInspector<TStack>>(vNStep)
    , fReportStack(vReportStack)
    , fE0(vE0)
    , fStartTime(std::chrono::system_clock::now()) {}

template <typename TStack>
StackInspector<TStack>::~StackInspector() {}

template <typename TStack>
process::EProcessReturn StackInspector<TStack>::DoStack(const TStack& vS) {
  [[maybe_unused]] int i = 0;
  HEPEnergyType Etot = 0_GeV;

  for (const auto& iterP : vS) {
    HEPEnergyType E = iterP.GetEnergy();
    Etot += E;
    if (fReportStack) {
      geometry::CoordinateSystem& rootCS = geometry::RootCoordinateSystem::GetInstance()
                                               .GetRootCoordinateSystem(); // for printout
      auto pos = iterP.GetPosition().GetCoordinates(rootCS);
      cout << "StackInspector: i=" << setw(5) << fixed << (i++) << ", id=" << setw(30)
           << iterP.GetPID() << " E=" << setw(15) << scientific << (E / 1_GeV) << " GeV, "
           << " pos=" << pos << " node = " << iterP.GetNode();
      if (iterP.GetPID() == Code::Nucleus) cout << " nuc_ref=" << iterP.GetNucleusRef();
      cout << endl;
    }
  }

  auto const now = std::chrono::system_clock::now();
  const std::chrono::duration<double> elapsed_seconds = now - fStartTime;
  std::time_t const now_time = std::chrono::system_clock::to_time_t(now);
  double const progress = (fE0 - Etot) / fE0;
  double const eta_seconds = elapsed_seconds.count() / progress;
  std::time_t const eta_time = std::chrono::system_clock::to_time_t(
      fStartTime + std::chrono::seconds((int)eta_seconds));

  cout << "StackInspector: "
       << " time=" << std::put_time(std::localtime(&now_time), "%T")
       << ", running=" << elapsed_seconds.count() << " seconds"
       << " (" << setw(3) << int(progress * 100) << "%)"
       << ", nStep=" << GetStep() << ", stackSize=" << vS.GetSize()
       << ", Estack=" << Etot / 1_GeV << " GeV"
       << ", ETA=" << std::put_time(std::localtime(&eta_time), "%T") << endl;
  return process::EProcessReturn::eOk;
}

template <typename TStack>
void StackInspector<TStack>::Init() {
  fReportStack = false;
  fStartTime = std::chrono::system_clock::now();
}

#include <corsika/cascade/testCascade.h>
#include <corsika/setup/SetupStack.h>

template class process::stack_inspector::StackInspector<setup::Stack>;
template class process::stack_inspector::StackInspector<TestCascadeStack>;
