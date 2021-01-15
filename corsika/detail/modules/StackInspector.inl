/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/StackInspector.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>

#include <corsika/setup/SetupTrajectory.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>

namespace corsika {

  template <typename TStack>
  StackInspector<TStack>::StackInspector(const int vNStep, const bool vReportStack,
                                         const HEPEnergyType vE0)
      : StackProcess<StackInspector<TStack>>(vNStep)
      , ReportStack_(vReportStack)
      , E0_(vE0)
      , StartTime_(std::chrono::system_clock::now()) {}

  template <typename TStack>
  StackInspector<TStack>::~StackInspector() {}

  template <typename TStack>
  void StackInspector<TStack>::doStack(const TStack& vS) {

    [[maybe_unused]] int i = 0;
    HEPEnergyType Etot = 0_GeV;

    for (const auto& iterP : vS) {
      HEPEnergyType E = iterP.getEnergy();
      Etot += E;
      if (ReportStack_) {
        CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem(); // for printout
        auto pos = iterP.getPosition().getCoordinates(rootCS);
        std::cout << "StackInspector: i=" << std::setw(5) << std::fixed << (i++)
                  << ", id=" << std::setw(30) << iterP.getPID() << " E=" << std::setw(15)
                  << std::scientific << (E / 1_GeV) << " GeV, "
                  << " pos=" << pos << " node = " << iterP.getNode();
        if (iterP.getPID() == Code::Nucleus)
          std::cout << " nuc_ref=" << iterP.getNucleusRef();
        std::cout << std::endl;
      }
    }

    auto const now = std::chrono::system_clock::now();
    const std::chrono::duration<double> elapsed_seconds = now - StartTime_;
    std::time_t const now_time = std::chrono::system_clock::to_time_t(now);
    auto const dE = E0_ - Etot;
    if (dE < dE_threshold_) return;
    double const progress = dE / E0_;

    double const eta_seconds = elapsed_seconds.count() / progress;
    std::time_t const eta_time = std::chrono::system_clock::to_time_t(
        StartTime_ + std::chrono::seconds((int)eta_seconds));

    std::cout << "StackInspector: "
              << " time=" << std::put_time(std::localtime(&now_time), "%T")
              << ", running=" << elapsed_seconds.count() << " seconds"
              << " (" << std::setw(3) << int(progress * 100) << "%)"
              << ", nStep=" << getStep() << ", stackSize=" << vS.getSize()
              << ", Estack=" << Etot / 1_GeV << " GeV"
              << ", ETA=" << std::put_time(std::localtime(&eta_time), "%T") << std::endl;
  }

} // namespace corsika