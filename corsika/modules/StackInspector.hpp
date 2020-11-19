/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/StackProcess.hpp>

#include <chrono>

namespace corsika::stack_inspector {

  template <typename TStack>
  class StackInspector : public corsika::StackProcess<StackInspector<TStack>> {

    typedef typename TStack::ParticleType Particle;

    using corsika::StackProcess<StackInspector<TStack>>::getStep;

  public:
    StackInspector(const int vNStep, const bool vReportStack, const HEPEnergyType vE0);
    ~StackInspector();

    void Init();
    void doStack(const TStack&);

    /**
     * To set a new E0, for example when a new shower event is started
     */
    void SetE0(const HEPEnergyType vE0) { E0_ = vE0; }

  private:
    bool ReportStack_;
    HEPEnergyType E0_;
    const HEPEnergyType dE_threshold_ = std::invoke([]() { return 1_eV; });
    decltype(std::chrono::system_clock::now()) StartTime_;
  };

} // namespace corsika::stack_inspector

#include <corsika/detail/modules/StackInspector.inl>
