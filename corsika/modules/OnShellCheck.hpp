/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

namespace corsika {

  class OnShellCheck : public SecondariesProcess<OnShellCheck> {

  public:
    OnShellCheck(const double vMassTolerance, const double vEnergyTolerance,
                 const bool vError);

    ~OnShellCheck();

    template <typename TView>
    void doSecondaries(TView&);

  private:
    // data members
    double average_shift_ = 0;
    double max_shift_ = 0;
    double count_ = 0;
    std::shared_ptr<spdlog::logger> logger_ = get_logger("on_shell_check");
    double mass_tolerance_;
    double energy_tolerance_;
    bool throw_error_;
  };
} // namespace corsika

#include <corsika/detail/modules/OnShellCheck.inl>
