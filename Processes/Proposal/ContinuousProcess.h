/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/proposal/ProposalProcessBase.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <unordered_map>

namespace corsika::process::proposal {

  //!
  //! Electro-magnetic and gamma continous losses produced by proposal. It makes
  //! use of interpolation tables which are runtime intensive calculation, but can be
  //! reused by setting the \param PROPOSAL::InterpolationDef::path_to_tables variable.
  //!
  class ContinuousProcess : public process::ContinuousProcess<ContinuousProcess>,
                            ProposalProcessBase {

    enum { eDISPLACEMENT, eSCATTERING };
    using calc_t = std::tuple<std::unique_ptr<PROPOSAL::Displacement>,
                              std::unique_ptr<PROPOSAL::Scattering>>;

    std::unordered_map<calc_key_t, calc_t, hash>
        calc; //!< Stores the displacement and scattering calculators.

    units::si::HEPEnergyType energy_lost_ = 0 * units::si::electronvolt;

    //!
    //! Build the displacement and scattering calculators and add it to calc.
    //!
    void BuildCalculator(particles::Code, environment::NuclearComposition const&) final;

  public:
    //!
    //! Produces the continuous loss calculator for leptons based on nuclear
    //! compositions and stochastic description limited by the particle cut.
    //!
    template <typename TEnvironment>
    ContinuousProcess(TEnvironment const&, corsika::units::si::HEPEnergyType _emCut);

    //!
    //! Multiple Scattering of the lepton. Stochastic deflection is not yet taken into
    //! account. Displacment of the track due to multiple scattering is not possible
    //! because of the constant referernce. The final direction will be updated anyway.
    //!
    template <typename Particle>
    void Scatter(Particle&, corsika::units::si::HEPEnergyType const&,
                 corsika::units::si::GrammageType const&);

    //!
    //! Produces the loss and deflection after given distance for the particle.
    //! If the particle if below the given energy threshold where it will be
    //! considered stochastically, it will be absorbed.
    //!
    template <typename Particle, typename Track>
    EProcessReturn DoContinuous(Particle&, Track const&);

    //!
    //! Calculates maximal step length of process.
    //!
    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle const&, Track const&);

    void showResults() const;
    void reset();
    corsika::units::si::HEPEnergyType energyLost() const { return energy_lost_; }
  };
} // namespace corsika::process::proposal
