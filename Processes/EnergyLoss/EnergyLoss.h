/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/ShowerAxis.h>
#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
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
    EnergyLoss(environment::ShowerAxis const& showerAxis);

    void Init() {}
    process::EProcessReturn DoContinuous(setup::Stack::ParticleType&,
                                         setup::Trajectory const&);
    units::si::LengthType MaxStepLength(setup::Stack::ParticleType const&,
                                        setup::Trajectory const&) const;
    units::si::HEPEnergyType GetTotal() const;
    void PrintProfile() const;
    static units::si::HEPEnergyType BetheBloch(setup::Stack::ParticleType const&,
                                               const units::si::GrammageType);
    static units::si::HEPEnergyType RadiationLosses(setup::Stack::ParticleType const&,
                                                    const units::si::GrammageType);
    static units::si::HEPEnergyType TotalEnergyLoss(setup::Stack::ParticleType const&,
                                                    const units::si::GrammageType);

  private:
    void FillProfile(setup::Trajectory const&, units::si::HEPEnergyType);

    units::si::GrammageType const dX_ = std::invoke([]() {
      using namespace units::si;
      return 10_g / square(1_cm);
    }); // profile binning
    environment::ShowerAxis const& shower_axis_;
    std::vector<units::si::HEPEnergyType> profile_; // longitudinal profile
  };

  units::si::GrammageType const dX_threshold_ = std::invoke([]() {
    using namespace units::si;
    return 0.0001_g / square(1_cm);
  });
} // namespace corsika::process::energy_loss
