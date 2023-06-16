/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/random/RNGManager.hpp>

#include <array>

namespace corsika::proposal {

  //!
  //! Particles which can be handled by proposal. That means they can be
  //! propagated and decayed if they decays.
  //!
  static constexpr std::array<Code, 7> tracked{
      Code::Photon, Code::Electron, Code::Positron, Code::MuMinus,
      Code::MuPlus, Code::TauPlus,  Code::TauMinus,
  };

  //!
  //! Relative EnergyCut value for PROPOSAL. Is defines that PROPOSAL will treat energy
  //! losses with a relative energy loss larger than v_cut stochastically.
  //!
  static constexpr double v_cut = 0.01;

  //!
  //! Internal map from particle codes to particle properties required for
  //! crosssections, decay and scattering algorithms. In the future the
  //! particles may be created by reading out the Corsica constants.
  //!
  static std::map<Code, PROPOSAL::ParticleDef> particle = {
      {Code::Photon, PROPOSAL::GammaDef()},   {Code::Electron, PROPOSAL::EMinusDef()},
      {Code::Positron, PROPOSAL::EPlusDef()}, {Code::MuMinus, PROPOSAL::MuMinusDef()},
      {Code::MuPlus, PROPOSAL::MuPlusDef()},  {Code::TauMinus, PROPOSAL::TauMinusDef()},
      {Code::TauPlus, PROPOSAL::TauPlusDef()}};

  //!
  //! Crosssection factories for different particle types.
  //!
  template <typename T>
  auto cross_builder = [](PROPOSAL::Medium& m,
                          corsika::units::si::HEPEnergyType
                              emCut) { //!< Stochastic losses smaller than the given cut
                                       //!< will be handeled continuously.
    auto particle_def = T();
    auto p_cut =
        std::make_shared<const PROPOSAL::EnergyCutSettings>(emCut / 1_MeV, v_cut, false);
    return PROPOSAL::GetStdCrossSections(particle_def, m, p_cut, true);
  };

  template <>
  auto cross_builder<PROPOSAL::GammaDef> =
      [](PROPOSAL::Medium& m,
         corsika::units::si::HEPEnergyType) { //!< Only-stochastic propagation
        auto p = PROPOSAL::GammaDef();
        auto interpolate = true;
        auto cross_vec = std::vector<std::shared_ptr<PROPOSAL::CrossSectionBase>>();
        auto photopair = PROPOSAL::make_crosssection(
            PROPOSAL::crosssection::PhotoPairKochMotz{false}, p, m, nullptr, interpolate);
        cross_vec.push_back(std::move(photopair));
        auto compton = PROPOSAL::make_crosssection(
            PROPOSAL::crosssection::ComptonKleinNishina{}, p, m, nullptr, interpolate);
        cross_vec.push_back(std::move(compton));
        auto photoproduction = PROPOSAL::make_crosssection(
            PROPOSAL::crosssection::PhotoproductionHeckC7Shadowing{}, p, m, nullptr,
            interpolate);
        cross_vec.push_back(std::move(photoproduction));
        auto photoeffect = PROPOSAL::make_crosssection(
            PROPOSAL::crosssection::PhotoeffectSauter{}, p, m, nullptr, interpolate);
        cross_vec.push_back(std::move(photoeffect));
        return cross_vec;
      };

  //!
  //! PROPOSAL default crosssections are maped to corresponding corsika particle
  //! code.
  //!
  static std::map<Code, std::function<PROPOSAL::crosssection_list_t(
                            PROPOSAL::Medium&, corsika::units::si::HEPEnergyType)>>
      cross = {{Code::Photon, cross_builder<PROPOSAL::GammaDef>},
               {Code::Electron, cross_builder<PROPOSAL::EMinusDef>},
               {Code::Positron, cross_builder<PROPOSAL::EPlusDef>},
               {Code::MuMinus, cross_builder<PROPOSAL::MuMinusDef>},
               {Code::MuPlus, cross_builder<PROPOSAL::MuPlusDef>},
               {Code::TauMinus, cross_builder<PROPOSAL::TauMinusDef>},
               {Code::TauPlus, cross_builder<PROPOSAL::TauPlusDef>}};

  //!
  //! PROPOSAL base process which handels mapping of particle codes to
  //! stored interpolation tables.
  //!
  class ProposalProcessBase {
  protected:
    default_prng_type& RNG_ = RNGManager<>::getInstance().getRandomStream("proposal");

    std::unordered_map<std::size_t, PROPOSAL::Medium>
        media; //!< maps nuclear composition from univers to media to produce
               //!< crosssections, which requires further ionization constants.

    //!
    //! Store cut and  nuclear composition of the whole universe in media which are
    //! required for creating crosssections by proposal.
    //!
    template <typename TEnvironment>
    ProposalProcessBase(TEnvironment const& _env);

    //!
    //! Checks if a particle can be processed by proposal
    //!
    bool canInteract(Code pcode) const;

    using calc_key_t = std::pair<std::size_t, Code>;

    //!
    //! Hash to store interpolation tables related to a pair of particle and nuclear
    //! composition.
    //!
    struct hash {
      size_t operator()(const calc_key_t& p) const noexcept;
    };

    //!
    //! Builds the calculator to the corresponding class
    //!
    virtual void buildCalculator(Code, NuclearComposition const&) = 0;

    //!
    //! Searches the particle dependet calculator dependent of actuall medium composition
    //! and particle type. If no calculator is found, the corresponding new calculator is
    //! built and then returned.
    //!
    template <typename Particle, typename Calculators>
    auto getCalculator(Particle& vP, Calculators& calc) {
      const auto& comp = vP.getNode()->getModelProperties().getNuclearComposition();
      auto calc_it = calc.find(std::make_pair(comp.getHash(), vP.getPID()));
      if (calc_it != calc.end()) return calc_it;
      buildCalculator(vP.getPID(), comp);
      return getCalculator(vP, calc);
    }
  };
} // namespace corsika::proposal

#include <corsika/detail/modules/proposal/ProposalProcessBase.inl>
