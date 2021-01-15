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
#include <corsika/media/ShowerAxis.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <map>

namespace corsika {

  /**
   *   PDG2018, passage of particles through matter
   *
   * Note, that \f$I_{\mathrm{eff}}\f$ of composite media a determined from \f$ \ln I =
   * \sum_i a_i \ln(I_i) \f$ where \f$ a_i \f$ is the fraction of the electron population
   * (\f$\sim Z_i\f$) of the \f$i\f$-th element. This can also be used for shell
   * corrections or density effects.
   *
   * The \f$I_{\mathrm{eff}}\f$ of compounds is not better than a few percent, if not
   * measured explicitly.
   *
   * For shell correction, see Sec 6 of https://www.nap.edu/read/20066/chapter/8#115
   *
   */

  class BetheBlochPDG : public ContinuousProcess<BetheBlochPDG> {

    using MeVgcm2 = decltype(1e6 * electronvolt / gram * square(1e-2 * meter));

  public:
    BetheBlochPDG(ShowerAxis const& showerAxis, HEPEnergyType emCut);

    ProcessReturn doContinuous(setup::Stack::particle_type&, setup::Trajectory const&);
    LengthType getMaxStepLength(setup::Stack::particle_type const&,
                                setup::Trajectory const&) const;
    static HEPEnergyType getBetheBloch(setup::Stack::particle_type const&,
                                       const GrammageType);
    static HEPEnergyType getRadiationLosses(setup::Stack::particle_type const&,
                                            const GrammageType);
    static HEPEnergyType getTotalEnergyLoss(setup::Stack::particle_type const&,
                                            const GrammageType);

    void showResults() const;
    void reset();
    HEPEnergyType getEnergyLost() const { return energy_lost_; }
    void printProfile() const;
    HEPEnergyType getTotal() const;

  private:
    void updateMomentum(corsika::setup::Stack::particle_type&, HEPEnergyType Enew);
    void fillProfile(setup::Trajectory const&, HEPEnergyType);

    GrammageType const dX_ = 10_g / square(1_cm); // profile binning
    GrammageType const dX_threshold_ = 0.0001_g / square(1_cm);
    ShowerAxis const& shower_axis_;
    HEPEnergyType emCut_;
    HEPEnergyType energy_lost_ = HEPEnergyType::zero();
    std::vector<HEPEnergyType> profile_; // longitudinal profile
  };

} // namespace corsika

#include <corsika/detail/modules/energy_loss/BetheBlochPDG.inl>
