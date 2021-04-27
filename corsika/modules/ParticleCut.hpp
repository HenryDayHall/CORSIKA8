/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <unordered_map>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/SecondariesProcess.hpp>
#include <corsika/framework/process/ContinuousProcess.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika {
  /**
     simple ParticleCut process. Goes through the secondaries of an interaction and
   removes particles according to their energy. Particles with a time delay of more than
   10ms are removed as well. Invisible particles (neutrinos) can be removed if selected.
   **/
  class ParticleCut : public SecondariesProcess<ParticleCut>,
                      public ContinuousProcess<ParticleCut> {

  public:
    /**
     * particle cut with energy thresholds for electrons, photons,
     *    hadrons (including nuclei with energy per nucleon) and muons
     *    invisible particles (neutrinos) can be cut or not
     **/
    ParticleCut(HEPEnergyType const eEleCut, HEPEnergyType const ePhoCut,
                HEPEnergyType const eHadCut, HEPEnergyType const eMuCut, bool const inv);

    //! simple cut. hadrons and muons are cut by threshold. EM particles are all
    //! discarded.
    ParticleCut(HEPEnergyType const eHadCut, HEPEnergyType const euCut, bool const inv);

    //! simplest cut. all particles have same threshold. EM particles can be set to be
    //! discarded altogether.
    ParticleCut(HEPEnergyType const eCut, bool const em, bool const inv);

    //! threshold for specific particles redefined. EM and invisible particles can be set
    //! to be discarded altogether.
    ParticleCut(std::unordered_map<Code const, HEPEnergyType const> const& eCuts,
                bool const em, bool const inv);

    void doSecondaries(corsika::setup::StackView&);
    ProcessReturn doContinuous(
        corsika::setup::Stack::particle_type& vParticle,
        corsika::setup::Trajectory const& vTrajectory,
        const bool limitFlag = false); // this is not used for ParticleCut
    LengthType getMaxStepLength(corsika::setup::Stack::particle_type const&,
                                corsika::setup::Trajectory const&) {
      return meter * std::numeric_limits<double>::infinity();
    }

    void printThresholds();
    void showResults(); // LCOV_EXCL_LINE
    void reset();

    HEPEnergyType getElectronECut() const { return get_energy_threshold(Code::Electron); }
    HEPEnergyType getPhotonECut() const { return get_energy_threshold(Code::Photon); }
    HEPEnergyType getMuonECut() const { return get_energy_threshold(Code::MuPlus); }
    HEPEnergyType getHadronECut() const { return get_energy_threshold(Code::Proton); }
    HEPEnergyType getInvEnergy() const { return inv_energy_; }
    HEPEnergyType getCutEnergy() const { return energy_; }
    HEPEnergyType getEmEnergy() const { return em_energy_; }
    unsigned int getNumberEmParticles() const { return em_count_; }
    unsigned int getNumberInvParticles() const { return inv_count_; }

  private:
    template <typename TParticle>
    bool checkCutParticle(TParticle const& p);

    template <typename TParticle>
    bool isBelowEnergyCut(TParticle const&) const;

    //! defines which particles are invisible, by default only neutrinos
    bool isInvisible(Code const&) const;

  private:
    bool doCutEm_;
    bool doCutInv_;
    HEPEnergyType energy_ = 0 * electronvolt;
    HEPEnergyType em_energy_ = 0 * electronvolt;
    unsigned int em_count_ = 0;
    HEPEnergyType inv_energy_ = 0 * electronvolt;
    unsigned int inv_count_ = 0;
  };

} // namespace corsika

#include <corsika/detail/modules/ParticleCut.inl>
