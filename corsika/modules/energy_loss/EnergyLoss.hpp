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

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/sequence/ContinuousProcess.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <map>

namespace corsika::energy_loss {

  class EnergyLoss : public corsika::ContinuousProcess<EnergyLoss> {

    using MeVgcm2 = decltype(1e6 * units::si::electronvolt / units::si::gram *
                             units::si::square(1e-2 * units::si::meter));

    void MomentumUpdate(setup::Stack::ParticleType&, units::si::HEPEnergyType Enew);

  public:
    template <typename TDim>
    EnergyLoss(corsika::Point const& injectionPoint,
               corsika::Vector<TDim> const& direction)
        : InjectionPoint_(injectionPoint)
        , ShowerAxisDirection_(direction.normalized()) {}

    EnergyLoss(setup::Trajectory const& trajectory)
        : EnergyLoss(trajectory.GetPosition(0), trajectory.GetV0()){};

    void Init() {}
    corsika::EProcessReturn DoContinuous(setup::Stack::ParticleType&,
                                         setup::Trajectory const&);
    units::si::LengthType MaxStepLength(setup::Stack::ParticleType const&,
                                        setup::Trajectory const&) const;
    units::si::HEPEnergyType GetTotal() const { return EnergyLossTot_; }
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

    units::si::HEPEnergyType EnergyLossTot_ = units::si::HEPEnergyType::zero();
    units::si::GrammageType const dX_ = std::invoke([]() {
      using namespace units::si;
      return 10_g / square(1_cm);
    });                                               // profile binning
    std::map<int, units::si::HEPEnergyType> Profile_; // longitudinal profile
    corsika::Point const InjectionPoint_;
    corsika::Vector<units::si::dimensionless_d> const ShowerAxisDirection_;
  };

  const units::si::GrammageType dX_threshold_ = std::invoke([]() {
    using namespace units::si;
    return 0.0001_g / square(1_cm);
  });
  
} // namespace corsika::energy_loss

#include <corsika/detail/modules/energy_loss/EnergyLoss.inl>
