/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::proposal {

  template <typename THadronicModel>
  template <typename TEnvironment>
  inline InteractionModel<THadronicModel>::InteractionModel(TEnvironment const& _env,
                                                            THadronicModel& hadint)
      : ProposalProcessBase(_env)
      , hadronicInteraction_(hadint) {}

  template <typename THadronicModel>
  inline void InteractionModel<THadronicModel>::buildCalculator(
      Code code, NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto const emCut = get_energy_production_threshold(
        code); //! energy resolutions globally defined for individual particles

    auto c = p_cross->second(media.at(comp.getHash()), emCut);

    // Look which interactions take place and build the corresponding
    // interaction and secondary builder. The interaction integral will
    // interpolated too and saved in the calc map by a key build out of a hash
    // of composed of the component and particle code.
    auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(c);
    calc_[std::make_pair(comp.getHash(), code)] = std::make_tuple(
        PROPOSAL::make_secondaries(inter_types, particle[code], media.at(comp.getHash())),
        PROPOSAL::make_interaction(c, true));
  }

  template <typename THadronicModel>
  template <typename TStackView>
  inline ProcessReturn InteractionModel<THadronicModel>::doInteraction(
      TStackView& view, Code const projectileId, FourMomentum const& projectileP4) {

    auto const projectile = view.getProjectile();

    if (canInteract(projectileId)) {

      // get or build corresponding calculators
      auto c = getCalculator(projectile, calc_);

      // get the rates of the interaction types for every component.
      std::uniform_real_distribution<double> distr(0., 1.);

      // sample a interaction-type, loss and component
      auto rates =
          std::get<eINTERACTION>(c->second)->Rates(projectile.getEnergy() / 1_MeV);
      auto [type, target_hash, v] = std::get<eINTERACTION>(c->second)->SampleLoss(
          projectile.getEnergy() / 1_MeV, rates, distr(RNG_));

      // TODO: This should become obsolete as soon #482 is fixed
      if (type == PROPOSAL::InteractionType::Undefined) {
        CORSIKA_LOG_WARN(
            "PROPOSAL: No particle interaction possible. "
            "Put initial particle back on stack.");
        view.addSecondary(std::make_tuple(projectileId, projectile.getEnergy(),
                                          projectile.getDirection()));
        return ProcessReturn::Ok;
      }

      // Read how much random numbers are required to calculate the secondaries.
      // Calculate the secondaries and deploy them on the corsika stack.
      auto rnd = std::vector<double>(
          std::get<eSECONDARIES>(c->second)->RequiredRandomNumbers(type));
      for (auto& it : rnd) it = distr(RNG_);
      Point const& place = projectile.getPosition();
      CoordinateSystemPtr const& labCS = place.getCoordinateSystem();

      auto point = PROPOSAL::Cartesian3D(
          place.getX(labCS) / 1_cm, place.getY(labCS) / 1_cm, place.getZ(labCS) / 1_cm);
      auto projectile_dir = projectile.getDirection();
      auto d = projectile_dir.getComponents(labCS);
      auto direction = PROPOSAL::Cartesian3D(d.getX().magnitude(), d.getY().magnitude(),
                                             d.getZ().magnitude());

      auto loss = PROPOSAL::StochasticLoss(
          static_cast<int>(type), v * projectile.getEnergy() / 1_MeV, point, direction,
          projectile.getTime() / 1_s, 0., projectile.getEnergy() / 1_MeV);
      PROPOSAL::Component target;
      if (type != PROPOSAL::InteractionType::Ioniz)
        target = PROPOSAL::Component::GetComponentForHash(target_hash);
      if (type == PROPOSAL::InteractionType::Photonuclear)
        CORSIKA_LOG_INFO(
            "HE photo-hadronic interaction! calling hadronic interaction model. Energy = "
            "{} GeV, v = {}, v * E = {}",
            projectile.getEnergy() / 1_GeV, v, v * projectile.getEnergy());
      if (type == PROPOSAL::InteractionType::Photonuclear &&
          projectile.getEnergy() > heHadronicModelThresholdLab_) {
        Code const targetId =
            get_nucleus_code(target.GetAtomicNum(), target.GetNucCharge());
        doHadronicInteraction(view, labCS, projectileP4, targetId);
      } else {
        auto sec =
            std::get<eSECONDARIES>(c->second)->CalculateSecondaries(loss, target, rnd);
        for (auto& s : sec) {
          auto E = s.energy * 1_MeV;
          auto vecProposal = s.direction;
          auto dir = DirectionVector(
              labCS, {vecProposal.GetX(), vecProposal.GetY(), vecProposal.GetZ()});
          auto sec_code = convert_from_PDG(static_cast<PDGCode>(s.type));
          view.addSecondary(std::make_tuple(sec_code, E - get_mass(sec_code), dir));
        }
      }
    }
    return ProcessReturn::Ok;
  }

  template <typename THadronicModel>
  template <typename TStackView>
  inline ProcessReturn InteractionModel<THadronicModel>::doHadronicInteraction(
      TStackView& view, CoordinateSystemPtr const& labCS,
      FourMomentum const& projectileP4, Code const& targetId) {
    CORSIKA_LOG_INFO(
        "HE photo-hadronic interaction! calling hadronic interaction model..");

    //  copy from sibyll::NuclearInteractionModel
    //  temporarily add to stack, will be removed after interaction in DoInteraction
    typename TStackView::inner_stack_value_type photonStack;
    Point const pDummy(labCS, {0_m, 0_m, 0_m});
    TimeType const tDummy = 0_ns;
    Code const hadPhotonCode = Code::Rho0; // stand in for hadronic-photon
    // auto const A = target.GetAtomicNum();
    // auto const Z = target.GetNucCharge();
    // Code const targetId = get_nucleus_code(A, Z);
    // target at rest
    FourMomentum const targetP4(get_mass(targetId),
                                MomentumVector(labCS, {0_GeV, 0_GeV, 0_GeV}));
    auto const p3PhotonLab = projectileP4.getSpaceLikeComponents();
    HEPEnergyType const Ekin = projectileP4.getTimeLikeComponent();
    auto hadronicPhoton = photonStack.addParticle(
        std::make_tuple(hadPhotonCode, Ekin, p3PhotonLab.normalized(), pDummy, tDummy));
    hadronicPhoton.setNode(view.getProjectile().getNode());
    CORSIKA_LOG_INFO("gam - had interaction: kin. energy of gamma: {} GeV", Ekin / 1_GeV);

    // create inelastic interaction of the hadronic photon
    // create new StackView for the photon
    TStackView photon_secondaries(hadronicPhoton);

    // call inner hadronic event generator
    CORSIKA_LOG_TRACE("calling HadronicInteraction...");
    CORSIKA_LOG_INFO("{} + {} interactions. Ekinlab = {}", hadPhotonCode, targetId,
                     Ekin / 1_GeV);
    hadronicInteraction_.doInteraction(photon_secondaries, hadPhotonCode, targetId,
                                       projectileP4, targetP4);
    for (const auto& pSec : photon_secondaries) {

      auto const p3lab = pSec.getMomentum();
      Code const pid = pSec.getPID();
      HEPEnergyType const mass = get_mass(pid);
      HEPEnergyType const Ekin = sqrt(p3lab.getSquaredNorm() + mass * mass) - mass;
      view.addSecondary(std::make_tuple(pid, Ekin, p3lab.normalized()));
    }
    CORSIKA_LOG_INFO("number of particles produced: {}", view.getEntries());
    return ProcessReturn::Ok;
  }

  template <typename THadronicModel>
  template <typename TParticle>
  inline CrossSectionType InteractionModel<THadronicModel>::getCrossSection(
      TParticle const& projectile, Code const projectileId,
      FourMomentum const& projectileP4) {

    // ==============================================
    // this block better diappears. RU 26.10.2021
    //
    // determine the volume where the particle is (last) known to be
    auto const* currentLogicalNode = projectile.getNode();
    NuclearComposition const& composition =
        currentLogicalNode->getModelProperties().getNuclearComposition();
    auto const meanMass = composition.getAverageMassNumber() * constants::u;
    // ==============================================

    if (canInteract(projectileId)) {
      auto c = getCalculator(projectile, calc_);
      return meanMass / (std::get<eINTERACTION>(c->second)->MeanFreePath(
                             projectileP4.getTimeLikeComponent() / 1_MeV) *
                         1_g / (1_cm * 1_cm));
    }

    return CrossSectionType::zero();
  }
} // namespace corsika::proposal
