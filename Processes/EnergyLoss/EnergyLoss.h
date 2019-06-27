/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _Processes_EnergyLoss_h_
#define _Processes_EnergyLoss_h_

#include <corsika/process/ContinuousProcess.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <map>

namespace corsika::process::energy_loss {

  class EnergyLoss : public corsika::process::ContinuousProcess<EnergyLoss> {

    using MeVgcm2 = decltype(1e6 * units::si::electronvolt / units::si::gram *
                             units::si::square(1e-2 * units::si::meter));

    void MomentumUpdate(setup::Stack::ParticleType&, units::si::HEPEnergyType Enew);

  public:
    EnergyLoss();
    void Init() {}
    process::EProcessReturn DoContinuous(setup::Stack::ParticleType&,
                                         setup::Trajectory const&);
    units::si::LengthType MaxStepLength(setup::Stack::ParticleType const&,
                                        setup::Trajectory const&) const;

    units::si::HEPEnergyType GetTotal() const { return fEnergyLossTot; }
    void PrintProfile() const;
    static units::si::HEPEnergyType BetheBloch(setup::Stack::ParticleType const&,
                                               const units::si::GrammageType);
    static units::si::HEPEnergyType RadiationLosses(setup::Stack::ParticleType const&,
                                                    const units::si::GrammageType);
    static units::si::HEPEnergyType TotalEnergyLoss(setup::Stack::ParticleType const&,
                                                    const units::si::GrammageType);

  private:
    int GetXbin(setup::Stack::ParticleType const&, setup::Trajectory const&,
                units::si::HEPEnergyType);

    units::si::HEPEnergyType fEnergyLossTot;
    units::si::GrammageType fdX;    // profile binning
    std::map<int, double> fProfile; // longitudinal profile
  };

} // namespace corsika::process::energy_loss

#endif
