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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>

namespace corsika {

  template <typename TStack>
  inline StackInspector<TStack>::StackInspector(int const vNStep, bool const vReportStack,
                                                HEPEnergyType const vE0)
      : StackProcess<StackInspector<TStack>>(vNStep)
      , ReportStack_(vReportStack)
      , E0_(vE0)
      , StartTime_(std::chrono::system_clock::now()) {}

  template <typename TStack>
  inline StackInspector<TStack>::~StackInspector() {}

  template <typename TStack>
  inline void StackInspector<TStack>::doStack(TStack const& vS) {

    [[maybe_unused]] int i = 0;
    HEPEnergyType Etot = 0_GeV;

    for (auto const& iterP : vS) {
      HEPEnergyType E = iterP.getEnergy();
      Etot += E;
      if (ReportStack_) {
        CoordinateSystemPtr const& rootCS = get_root_CoordinateSystem(); // for printout
        auto pos = iterP.getPosition().getCoordinates(rootCS);
        CORSIKA_LOG_INFO(
            "StackInspector: i= {:5d}"
            ", id= {:^15}"
            " E={:15e} GeV, "
            " pos= {}"
            " node = {}",
            (i++), iterP.getPID(), (E / 1_GeV), pos, fmt::ptr(iterP.getNode()));
      }
    }

    std::chrono::system_clock::time_point const now = std::chrono::system_clock::now();
    std::chrono::duration<double> const elapsed_seconds = now - StartTime_; // seconds
    auto const dE = E0_ - Etot;
    if (dE < dE_threshold_) return;
    double const progress = dE / E0_;

    // for printout
    std::time_t const now_time = std::chrono::system_clock::to_time_t(now);
    std::time_t const start_time = std::chrono::system_clock::to_time_t(StartTime_);

    if (progress > 0) {

      double const eta_seconds = elapsed_seconds.count() / progress;
      std::chrono::system_clock::time_point const eta =
          StartTime_ + std::chrono::seconds((int)eta_seconds);

      // for printout
      std::time_t const eta_time = std::chrono::system_clock::to_time_t(eta);

      int const yday0 = std::localtime(&start_time)->tm_yday;
      int const yday1 = std::localtime(&eta_time)->tm_yday;
      int const dyday = yday1 - yday0;

      CORSIKA_LOG_INFO(
          "StackInspector: "
          " time={}"
          ", running={} seconds"
          " ( {:.1f}%)"
          ", nStep={}"
          ", stackSize={}"
          ", Estack={} GeV"
          ", ETA={}{}",
          std::put_time(std::localtime(&now_time), "%T"), elapsed_seconds.count(),
          (progress * 100), getStep(), vS.getSize(), Etot / 1_GeV,
          (dyday == 0 ? "" : fmt::format("+{}d ", dyday)),
          std::put_time(std::localtime(&eta_time), "%T"));
    } else {
      CORSIKA_LOG_INFO(
          "StackInspector: "
          " time={}"
          ", running={} seconds"
          " ( {:.1f}%)"
          ", nStep={}"
          ", stackSize={}"
          ", Estack={} GeV"
          ", ETA={}{}",
          std::put_time(std::localtime(&now_time), "%T"), elapsed_seconds.count(),
          (progress * 100), getStep(), vS.getSize(), Etot / 1_GeV, "---", "---");
    }
  }

} // namespace corsika
