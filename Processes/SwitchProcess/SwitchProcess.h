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
   * SwitchProcess itself will always be regarded as a ProcessSequence rather than an
   * InteractionProcess when assembled into a greater ProcessSequence.
   */

  template <class TLowEProcess, class THighEProcess>
  class SwitchProcess
      : public InteractionProcess<SwitchProcess<TLowEProcess, THighEProcess>> {
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
          return fLowEProcess.DoInteraction(vS);
        }
      } else {
        if constexpr (is_process_sequence_v<THighEProcess>) {
          return fHighEProcess.SelectInteraction(vP, vS, lambda_select, lambda_inv_count);
        } else {
          return fHighEProcess.DoInteraction(vS);
        }
      }
    }
  };
} // namespace corsika::process::switch_process

template <typename A, typename B>
struct corsika::process::is_process_sequence<
    corsika::process::switch_process::SwitchProcess<A, B>> : std::true_type {};

#endif
