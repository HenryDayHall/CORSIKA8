/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/process/StackProcess.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <chrono>

namespace corsika::process {

  namespace stack_inspector {

    template <typename TStack>
    class StackInspector : public corsika::process::StackProcess<StackInspector<TStack>> {

      typedef typename TStack::ParticleType Particle;

      using corsika::process::StackProcess<StackInspector<TStack>>::GetStep;

    public:
      StackInspector(const int vNStep, const bool vReportStack,
                     const corsika::units::si::HEPEnergyType vE0);
      ~StackInspector();
      
      void DoStack(const TStack&);

      /**
       * To set a new E0, for example when a new shower event is started
       */
      void SetE0(const corsika::units::si::HEPEnergyType vE0) { E0_ = vE0; }

    private:
      bool ReportStack_;
      corsika::units::si::HEPEnergyType E0_;
      const corsika::units::si::HEPEnergyType dE_threshold_ = std::invoke([]() {
        using namespace units::si;
        return 1_eV;
      });
      decltype(std::chrono::system_clock::now()) StartTime_;
    };

  } // namespace stack_inspector

} // namespace corsika::process
