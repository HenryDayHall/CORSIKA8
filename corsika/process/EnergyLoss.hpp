/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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

#include <map>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {

  class EnergyLoss : public corsika::ContinuousProcess<EnergyLoss> {

    using MeVgcm2 = decltype(1e6 * units::si::electronvolt / units::si::gram *
                             units::si::square(1e-2 * units::si::meter));

    void MomentumUpdate(setup::Stack::ParticleType&, units::si::HEPEnergyType Enew);

  public:

    template <typename TDim>
    EnergyLoss(geometry::Point const& injectionPoint,
               geometry::Vector<TDim> const& direction)
        : InjectionPoint_(injectionPoint)
        , ShowerAxisDirection_(direction.normalized()) {}

    EnergyLoss(setup::Trajectory const& trajectory)
        : EnergyLoss(trajectory.GetPosition(0), trajectory.GetV0()){};

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

    void showResults() const;
    void reset();
    corsika::units::si::HEPEnergyType energyLost() const { return energy_lost_; }

  private:
    void FillProfile(setup::Trajectory const&, units::si::HEPEnergyType);

    units::si::GrammageType const dX_ = std::invoke([]() {
      using namespace units::si;
      return 10_g / square(1_cm);
    }); // profile binning

  private:
    environment::ShowerAxis const& shower_axis_;
    corsika::units::si::HEPEnergyType emCut_;
    std::vector<units::si::HEPEnergyType> profile_; // longitudinal profile
    units::si::HEPEnergyType energy_lost_ = 0 * units::si::electronvolt;
  };

  units::si::GrammageType const dX_threshold_ = std::invoke([]() {
    using namespace units::si;
    return 0.0001_g / square(1_cm);
  });
} // namespace corsika

