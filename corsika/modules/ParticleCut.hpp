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

#include <corsika/modules/writers/WriterOff.hpp>

namespace corsika {
  /**
     simple ParticleCut process. Goes through the secondaries of an interaction and
   removes particles according to their kinetic energy. Particles with a time delay of
  more than 10ms are removed as well. Invisible particles (neutrinos) can be removed if
  selected. The threshold value is set to 0 by default but in principle can be configured
  for each particle. Special constructors for cuts by the following groups are
  implemented: (electrons,positrons), photons, hadrons and muons.
   **/
  template <typename TOutput = WriterOff>
  class ParticleCut : public SecondariesProcess<ParticleCut<TOutput>>,
                      public ContinuousProcess<ParticleCut<TOutput>>,
                      public TOutput {

  public:
    /**
     * particle cut with kinetic energy thresholds for electrons, photons,
     *    hadrons (including nuclei with energy per nucleon) and muons
     *    invisible particles (neutrinos) can be cut or not
     */
    template <typename... TArgs>
    ParticleCut(HEPEnergyType const eEleCut, HEPEnergyType const ePhoCut,
                HEPEnergyType const eHadCut, HEPEnergyType const eMuCut, bool const inv,
                TArgs&&... args);

    /**
     * Threshold for specific particles redefined. EM and invisible particles can be set
     * to be discarded altogether.
     */
    ParticleCut(std::unordered_map<Code const, HEPEnergyType const> const& eCuts,
                bool const inv);

    template <typename TStackView>
    void doSecondaries(TStackView&);

    template <typename TParticle, typename TTrajectory>
    ProcessReturn doContinuous(
        TParticle& vParticle, TTrajectory const& vTrajectory,
        const bool limitFlag = false); // this is not used for ParticleCut

    template <typename TParticle, typename TTrajectory>
    LengthType getMaxStepLength(TParticle const&, TTrajectory const&) {
      return meter * std::numeric_limits<double>::infinity();
    }

    void printThresholds() const;
    void showResults() const; // LCOV_EXCL_LINE
    void reset();

    HEPEnergyType getElectronKineticECut() const {
      return calculate_kinetic_energy_threshold(Code::Electron);
    }
    HEPEnergyType getPhotonKineticECut() const {
      return calculate_kinetic_energy_threshold(Code::Photon);
    }
    HEPEnergyType getMuonKineticECut() const {
      return calculate_kinetic_energy_threshold(Code::MuPlus);
    }
    HEPEnergyType getHadronKineticECut() const {
      return calculate_kinetic_energy_threshold(Code::Proton);
    }
    //! returns total energy of particles that were removed by cut for invisible particles
    HEPEnergyType getInvEnergy() const { return energy_invcut_; }
    //! returns total energy of particles that were removed by cut in time
    HEPEnergyType getTimeCutEnergy() const { return energy_timecut_; }
    //! returns total energy of particles that were removed by cut in kinetic energy
    HEPEnergyType getCutEnergy() const { return energy_cut_; }
    //! returns number of invisible particles
    unsigned int getNumberInvParticles() const { return inv_count_; }

    // get configuration of this node
    YAML::Node getConfig() const override;

  private:
    template <typename TParticle>
    bool checkCutParticle(TParticle const& p);

    template <typename TParticle>
    bool isBelowEnergyCut(TParticle const&) const;

    //! defines which particles are invisible, by default only neutrinos
    bool isInvisible(Code const&) const;

  private:
    bool doCutInv_;
    HEPEnergyType energy_cut_ = 0 * electronvolt;
    HEPEnergyType energy_timecut_ = 0 * electronvolt;
    HEPEnergyType energy_invcut_ = 0 * electronvolt;
    unsigned int inv_count_ = 0;
    unsigned int energy_count_ = 0;

    HEPEnergyType energy_event_; // per event sum
  };

} // namespace corsika

#include <corsika/detail/modules/ParticleCut.inl>
