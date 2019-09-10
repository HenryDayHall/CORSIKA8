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
    template <typename TDim>
    EnergyLoss(geometry::Point const& injectionPoint,
               geometry::Vector<TDim> const& direction)
        : fInjectionPoint(injectionPoint)
        , fShowerAxisDirection(direction.normalized()) {}

    EnergyLoss(setup::Trajectory const& trajectory)
        : EnergyLoss(trajectory.GetPosition(0), trajectory.GetV0()){};

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
    void FillProfile(setup::Stack::ParticleType const&, setup::Trajectory const&,
                     units::si::HEPEnergyType);
    // void FillProfileAbsorbed(setup::Stack::ParticleType const&, setup::Trajectory
    // const&);

    units::si::HEPEnergyType fEnergyLossTot = units::si::HEPEnergyType::zero();
    units::si::GrammageType const fdX = std::invoke([]() {
      using namespace units::si;
      return 10_g / square(1_cm);
    });                                               // profile binning
    std::map<int, units::si::HEPEnergyType> fProfile; // longitudinal profile
    geometry::Point const fInjectionPoint;
    geometry::Vector<units::si::dimensionless_d> const fShowerAxisDirection;
  };

} // namespace corsika::process::energy_loss

#endif
