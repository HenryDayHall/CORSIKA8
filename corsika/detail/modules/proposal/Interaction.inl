/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/modules/proposal/Interaction.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::proposal {

  template <>
  inline Interaction::Interaction(setup::Environment const& _env)
      : ProposalProcessBase(_env) {}

  inline void Interaction::buildCalculator(Code code, NuclearComposition const& comp) {
    // search crosssection builder for given particle
    auto p_cross = cross.find(code);
    if (p_cross == cross.end())
      throw std::runtime_error("PROPOSAL could not find corresponding builder");

    // interpolate the crosssection for given media and energy cut. These may
    // take some minutes if you have to build the tables and cannot read the
    // from disk
    auto const emCut =
        get_kinetic_energy_threshold(code) +
        get_mass(code); //! energy thresholds globally defined for individual particles

    auto c = p_cross->second(media.at(comp.getHash()), emCut);

    // Look which interactions take place and build the corresponding
    // interaction and secondarie builder. The interaction integral will
    // interpolated too and saved in the calc map by a key build out of a hash
    // of composed of the component and particle code.
    auto inter_types = PROPOSAL::CrossSectionVector::GetInteractionTypes(c);
    calc[std::make_pair(comp.getHash(), code)] = std::make_tuple(
        PROPOSAL::make_secondaries(inter_types, particle[code], media.at(comp.getHash())),
        PROPOSAL::make_interaction(c, true));
  }

  template <>
  inline ProcessReturn Interaction::doInteraction(setup::StackView& view) {

    auto const projectile = view.getProjectile();

    if (canInteract(projectile.getPID())) {

      // get or build corresponding calculators
      auto c = getCalculator(projectile, calc);

      // get the rates of the interaction types for every component.
      std::uniform_real_distribution<double> distr(0., 1.);

      // sample a interaction-type, loss and component
      auto rates =
          std::get<eINTERACTION>(c->second)->Rates(projectile.getEnergy() / 1_MeV);
      auto [type, target_hash, v] = std::get<eINTERACTION>(c->second)->SampleLoss(
          projectile.getEnergy() / 1_MeV, rates, distr(RNG_));

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
      auto target = PROPOSAL::Component::component_map->operator[](target_hash);
      auto sec =
          std::get<eSECONDARIES>(c->second)->CalculateSecondaries(loss, target, rnd);
      for (auto& s : sec) {
        auto E = s.energy * 1_MeV;
        auto vecProposal = s.direction;
        auto vec = QuantityVector(vecProposal.GetX() * E, vecProposal.GetY() * E,
                                  vecProposal.GetZ() * E);
        auto p = MomentumVector(labCS, vec);
        auto sec_code = convert_from_PDG(static_cast<PDGCode>(s.type));
        view.addSecondary(
            std::make_tuple(sec_code, p, projectile.getPosition(), projectile.getTime()));
      }
    }
    return ProcessReturn::Ok;
  }

  template <>
  inline GrammageType Interaction::getInteractionLength(
      setup::Stack::particle_type const& projectile) {

    if (canInteract(projectile.getPID())) {
      auto c = getCalculator(projectile, calc);
      return std::get<eINTERACTION>(c->second)->MeanFreePath(projectile.getEnergy() /
                                                             1_MeV) *
             1_g / (1_cm * 1_cm);
    }
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }
} // namespace corsika::proposal
