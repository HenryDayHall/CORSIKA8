#pragma once

#include <PROPOSAL/PROPOSAL.h>

#include <corsika/particles/ParticleProperties.h>
#include <corsika/random/RNGManager.h>
#include <corsika/setup/SetupEnvironment.h>
#include <array>

namespace corsika::process::proposal {

  //!
  //! Particles which can be handled by proposal. That means they can be
  //! propagated and decayed if they decays.
  //!
  static constexpr std::array<particles::Code, 7> tracked{
      particles::Code::Gamma,    particles::Code::Electron, particles::Code::Positron,
      particles::Code::MuMinus,  particles::Code::MuPlus,   particles::Code::TauPlus,
      particles::Code::TauMinus,
  };

  //!
  //! Internal map from particle codes to particle properties required for
  //! crosssections, decay and scattering algorithms. In the future the
  //! particles may be created by reading out the Corsica constants.
  //!
  static std::map<particles::Code, PROPOSAL::ParticleDef> particle = {
      {particles::Code::Gamma, PROPOSAL::GammaDef()},
      {particles::Code::Electron, PROPOSAL::EMinusDef()},
      {particles::Code::Positron, PROPOSAL::EPlusDef()},
      {particles::Code::MuMinus, PROPOSAL::MuMinusDef()},
      {particles::Code::MuPlus, PROPOSAL::MuPlusDef()},
      {particles::Code::TauMinus, PROPOSAL::TauMinusDef()},
      {particles::Code::TauPlus, PROPOSAL::TauPlusDef()}};

  //!
  //! Crosssection factories for different particle types.
  //!
  template <typename T>
  static auto cross_builder =
      [](PROPOSAL::Medium& m, corsika::units::si::HEPEnergyType emCut) {
        using namespace corsika::units::si;
        auto p_cut =
            std::make_shared<const PROPOSAL::EnergyCutSettings>(emCut / 1_MeV, 1, true);
        return PROPOSAL::DefaultCrossSections<T>::template Get<std::false_type>(
            T(), m, p_cut, true);
      };

  //!
  //! PROPOSAL default crosssections are maped to corresponding corsika particle
  //! code.
  //!
  static std::map<particles::Code,
                  std::function<PROPOSAL::crosssection_list_t<PROPOSAL::ParticleDef,
                                                              PROPOSAL::Medium>(
                      PROPOSAL::Medium&, corsika::units::si::HEPEnergyType)>>
      cross = {{particles::Code::Gamma, cross_builder<PROPOSAL::GammaDef>},
               {particles::Code::Electron, cross_builder<PROPOSAL::EMinusDef>},
               {particles::Code::Positron, cross_builder<PROPOSAL::EPlusDef>},
               {particles::Code::MuMinus, cross_builder<PROPOSAL::MuMinusDef>},
               {particles::Code::MuPlus, cross_builder<PROPOSAL::MuPlusDef>},
               {particles::Code::TauMinus, cross_builder<PROPOSAL::TauMinusDef>},
               {particles::Code::TauPlus, cross_builder<PROPOSAL::TauPlusDef>}};

  //!
  //! PROPOSAL base process which handels mapping of particle codes to
  //! stored interpolation tables.
  //!
  class ProposalProcessBase {
  protected:
    corsika::units::si::HEPEnergyType
        emCut_;                 //!< Stochastic losses smaller than the given cut
                                //!< will be handeled continuously.
    corsika::random::RNG& fRNG; //!< random number generator used by proposal

    std::unordered_map<std::size_t, PROPOSAL::Medium>
        media; //!< maps nuclear composition from univers to media to produce
               //!< crosssections, which requires further ionization constants.

    //!
    //! Store cut and  nuclear composition of the whole universe in media which are
    //! required for creating crosssections by proposal.
    //!
    ProposalProcessBase(corsika::setup::SetupEnvironment const& _env,
                        corsika::units::si::HEPEnergyType _emCut);

    //!
    //! Checks if a particle can be processed by proposal
    //!
    bool CanInteract(particles::Code pcode) const;

    using calc_key_t = std::pair<std::size_t, particles::Code>;

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
    virtual void BuildCalculator(particles::Code,
                                 environment::NuclearComposition const&) = 0;

    //!
    //! Searches the particle dependet calculator dependent of actuall medium composition
    //! and particle type. If no calculator is found, the corresponding new calculator is
    //! built and then returned.
    //!
    template <typename Particle, typename Calculators>
    auto GetCalculator(Particle& vP, Calculators& calc) {
      const auto& comp = vP.GetNode()->GetModelProperties().GetNuclearComposition();
      auto calc_it = calc.find(std::make_pair(comp.hash(), vP.GetPID()));
      if (calc_it != calc.end()) return calc_it;
      BuildCalculator(vP.GetPID(), comp);
      return GetCalculator(vP, calc);
    }
  };
} // namespace corsika::process::proposal
