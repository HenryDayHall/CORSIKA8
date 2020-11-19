/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <map>

namespace corsika::energy_loss {

  class BetheBlochPDG : public corsika::ContinuousProcess<BetheBlochPDG> {

    using MeVgcm2 = decltype(1e6 * electronvolt / gram * square(1e-2 * meter));

    void MomentumUpdate(setup::Stack::ParticleType&, HEPEnergyType Enew);

  public:
    template <typename TDim>
    BetheBlochPDG(Point const& injectionPoint, Vector<TDim> const& direction)
        : InjectionPoint_(injectionPoint)
        , ShowerAxisDirection_(direction.normalized()) {}

    BetheBlochPDG(setup::Trajectory const& trajectory)
        : BetheBlochPDG(trajectory.GetPosition(0), trajectory.GetV0()){};

    void Init() {}
    ProcessReturn doContinuous(setup::Stack::ParticleType&, setup::Trajectory const&);
    LengthType MaxStepLength(setup::Stack::ParticleType const&,
                             setup::Trajectory const&) const;
    HEPEnergyType GetTotal() const { return BetheBlochPDGTot_; }
    void PrintProfile() const;
    static HEPEnergyType BetheBloch(setup::Stack::ParticleType const&,
                                    const GrammageType);
    static HEPEnergyType RadiationLosses(setup::Stack::ParticleType const&,
                                         const GrammageType);
    static HEPEnergyType TotalEnergyLoss(setup::Stack::ParticleType const&,
                                         const GrammageType);

  private:
    void FillProfile(setup::Stack::ParticleType const&, setup::Trajectory const&,
                     HEPEnergyType);

    HEPEnergyType BetheBlochPDGTot_ = HEPEnergyType::zero();
    std::map<int, HEPEnergyType> Profile_; // longitudinal profile
    corsika::Point const InjectionPoint_;
    corsika::Vector<dimensionless_d> const ShowerAxisDirection_;
    GrammageType const dX_ = 10_g / square(1_cm); // profile binning
    const GrammageType dX_threshold_ = 0.0001_g / square(1_cm);
  };

} // namespace corsika::energy_loss

#include <corsika/detail/modules/energy_loss/BetheBlochPDG.inl>
