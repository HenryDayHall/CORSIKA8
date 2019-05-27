/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_SwitchProcess_h
#define _corsika_SwitchProcess_h

#include <corsika/process/InteractionProcess.h>
#include <corsika/process/ProcessSequence.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/units/PhysicalUnits.h>

namespace corsika::process::switch_process {

  /**
   * This process provides an energy-based switch between two interaction processes P1 and
   * P1. For energies below the threshold, P1 is invoked, otherwise P2. Both can be either
   * single interaction processes or multiple ones combined in a ProcessSequence. A
   * SwitchProcess itself will always be regarded as a distinct case when assembled into a
   * (greater) ProcessSequence.
   */

  template <class TLowEProcess, class THighEProcess>
  class SwitchProcess : public BaseProcess<SwitchProcess<TLowEProcess, THighEProcess>> {
    TLowEProcess& fLowEProcess;
    THighEProcess& fHighEProcess;
    units::si::HEPEnergyType const fThresholdEnergy;

  public:
    SwitchProcess(TLowEProcess& vLowEProcess, THighEProcess& vHighEProcess,
                  units::si::HEPEnergyType vThresholdEnergy)
        : fLowEProcess(vLowEProcess)
        , fHighEProcess(vHighEProcess)
        , fThresholdEnergy(vThresholdEnergy) {}

    void Init() {
      fLowEProcess.Init();
      fHighEProcess.Init();
    }

    template <typename TParticle>
    corsika::units::si::InverseGrammageType GetInverseInteractionLength(TParticle& p) {
      return 1 / GetInteractionLength(p);
    }

    template <typename TParticle>
    units::si::GrammageType GetInteractionLength(TParticle& vParticle) {
      if (vParticle.GetEnergy() < fThresholdEnergy) {
        if constexpr (is_process_sequence_v<TLowEProcess>) {
          return fLowEProcess.GetTotalInteractionLength(vParticle);
        } else {
          return fLowEProcess.GetInteractionLength(vParticle);
        }
      } else {
        if constexpr (is_process_sequence_v<THighEProcess>) {
          return fHighEProcess.GetTotalInteractionLength(vParticle);
        } else {
          return fHighEProcess.GetInteractionLength(vParticle);
        }
      }
    }

    // required to conform to ProcessSequence interface. We cannot just
    // implement DoInteraction() because we want to call SelectInteraction
    // in case a member process is a ProcessSequence.
    template <typename TParticle, typename TSecondaries>
    EProcessReturn SelectInteraction(
        TParticle& vP, TSecondaries& vS,
        [[maybe_unused]] corsika::units::si::InverseGrammageType lambda_select,
        corsika::units::si::InverseGrammageType& lambda_inv_count) {
      if (vP.GetEnergy() < fThresholdEnergy) {
        if constexpr (is_process_sequence_v<TLowEProcess>) {
          return fLowEProcess.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
        } else {
          lambda_inv_count += fLowEProcess.GetInverseInteractionLength(vP);
          // check if we should execute THIS process and then EXIT
          if (lambda_select < lambda_inv_count) {
            fLowEProcess.DoInteraction(vS);
            return EProcessReturn::eInteracted;
          } else {
            return EProcessReturn::eOk;
          }
        }
      } else {
        if constexpr (is_process_sequence_v<THighEProcess>) {
          return fHighEProcess.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
        } else {
          lambda_inv_count += fHighEProcess.GetInverseInteractionLength(vP);
          // check if we should execute THIS process and then EXIT
          if (lambda_select < lambda_inv_count) {
            fHighEProcess.DoInteraction(vS);
            return EProcessReturn::eInteracted;
          } else {
            return EProcessReturn::eOk;
          }
        }
      }
    }
  };
} // namespace corsika::process::switch_process

#endif
