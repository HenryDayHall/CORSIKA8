
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

namespace corsika::process::EnergyLoss {

  class EnergyLoss : public corsika::process::ContinuousProcess<EnergyLoss> {

    using MeVgcm2 = decltype(1e6 * units::si::electronvolt / units::si::gram *
                             corsika::units::si::square(1e-2 * units::si::meter));

    void MomentumUpdate(corsika::setup::Stack::ParticleType&,
                        corsika::units::si::HEPEnergyType Enew);

  public:
    EnergyLoss();
    void Init() {}

    corsika::process::EProcessReturn DoContinuous(corsika::setup::Stack::ParticleType&,
                                                  corsika::setup::Trajectory&);
    corsika::units::si::LengthType MaxStepLength(corsika::setup::Stack::ParticleType&,
                                                 corsika::setup::Trajectory&);

    corsika::units::si::HEPEnergyType GetTotal() const { return fEnergyLossTot; }
    void PrintProfile() const;

  private:
    corsika::units::si::HEPEnergyType BetheBloch(
        corsika::setup::Stack::ParticleType& p,
        const corsika::units::si::GrammageType dX);

    int GetXbin(corsika::setup::Stack::ParticleType& p,
                const corsika::units::si::HEPEnergyType dE);

    corsika::units::si::HEPEnergyType fEnergyLossTot;
    corsika::units::si::GrammageType fdX; // profile binning
    std::map<int, double> fProfile;       // longitudinal profile
  };

} // namespace corsika::process::EnergyLoss

#endif
