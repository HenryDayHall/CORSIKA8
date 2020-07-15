/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _corsika_process_proposal_interaction_h_
#define _corsika_process_proposal_interaction_h_

#include <PROPOSAL/PROPOSAL.h>
#include <corsika/environment/Environment.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/ContinuousProcess.h>
#include <corsika/process/particle_cut/ParticleCut.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <unordered_map>
#include "PROPOSAL/PROPOSAL.h"

using std::unordered_map;

using namespace corsika::environment;
using namespace corsika::units::si;

using CORSIKA_ParticleCut = corsika::process::particle_cut::ParticleCut;

namespace corsika::process::proposal {

  class ContinuousProcess
      : public corsika::process::ContinuousProcess<ContinuousProcess> {
    CORSIKA_ParticleCut& cut;
    corsika::random::RNG& fRNG;
    static constexpr std::array<particles::Code, 7> tracked_particles{
        particles::Code::Gamma,    particles::Code::Electron, particles::Code::Positron,
        particles::Code::MuMinus,  particles::Code::MuPlus,   particles::Code::TauPlus,
        particles::Code::TauMinus,
    };
    unordered_map<const NuclearComposition*, PROPOSAL::Medium> media;

    bool CanInteract(particles::Code pcode) const noexcept;

    using calc_key_t = std::pair<const NuclearComposition*, particles::Code>;
    using calc_t =
        tuple<unique_ptr<PROPOSAL::Displacement>, unique_ptr<PROPOSAL::Scattering>>;

    struct disp_hash {
      size_t operator()(const calc_key_t& p) const {
        return std::hash<const NuclearComposition*>{}(p.first) ^
               std::hash<particles::Code>{}(p.second);
      }
    };

    enum { DISPLACEMENT, SCATTERING };
    unordered_map<calc_key_t, calc_t, disp_hash> calc;

    template <typename Particle>
    auto BuildCalculator(particles::Code code, Particle p_def,
                         NuclearComposition const& comp) {
      auto cross = GetStdCrossSections(
          p_def, media.at(&comp),
          make_shared<const PROPOSAL::EnergyCutSettings>(cut.GetECut() / 1_MeV, 1, false),
          true);
      auto [insert_it, success] =
          calc.insert({std::make_pair(&comp, code),
                       std::make_tuple(PROPOSAL::make_displacement(cross, true),
                                       PROPOSAL::make_scattering("highland", p_def,
                                                                 media.at(&comp)))});
      return insert_it;
    }

    auto BuildCalculator(particles::Code corsika_code, NuclearComposition const& comp) {
      if (corsika_code == particles::Code::Gamma)
        return BuildCalculator(particles::Code::Gamma, PROPOSAL::GammaDef(), comp);
      if (corsika_code == particles::Code::Electron)
        return BuildCalculator(particles::Code::Electron, PROPOSAL::EMinusDef(), comp);
      if (corsika_code == particles::Code::Positron)
        return BuildCalculator(particles::Code::Positron, PROPOSAL::EPlusDef(), comp);
      if (corsika_code == particles::Code::MuMinus)
        return BuildCalculator(particles::Code::MuMinus, PROPOSAL::MuMinusDef(), comp);
      if (corsika_code == particles::Code::MuPlus)
        return BuildCalculator(particles::Code::MuPlus, PROPOSAL::MuPlusDef(), comp);
      if (corsika_code == particles::Code::TauMinus)
        return BuildCalculator(particles::Code::TauMinus, PROPOSAL::TauMinusDef(), comp);
      if (corsika_code == particles::Code::TauPlus)
        return BuildCalculator(particles::Code::TauPlus, PROPOSAL::TauPlusDef(), comp);
      throw std::runtime_error("PROPOSAL could not find corresponding builder");
    }

    template <typename Particle>
    auto GetCalculator(Particle& vP) {
      auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calc.find(std::make_pair(&comp, vP.GetPID()));
      if (calc_it != calc.end()) return calc_it;
      return BuildCalculator(vP.GetPID(), comp);
    }

  public:
    template <typename TEnvironment>
    ContinuousProcess(TEnvironment const&, CORSIKA_ParticleCut&);

    void Init(){};

    template <typename Particle>
    corsika::units::si::HEPEnergyType TotalEnergyLoss(
        Particle const&, corsika::units::si::GrammageType const&);

    template <typename Particle>
    void Scatter(Particle&, corsika::units::si::HEPEnergyType const&,
                 corsika::units::si::GrammageType const&);

    template <typename Particle, typename Track>
    EProcessReturn DoContinuous(Particle&, Track const&);

    template <typename Particle, typename Track>
    corsika::units::si::LengthType MaxStepLength(Particle const&, Track const&);
  };
} // namespace corsika::process::proposal

#endif
