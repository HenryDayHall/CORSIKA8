/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */


#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/proposal/Interaction.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <limits>
#include <memory>
#include <random>
#include <tuple>

using Component_PROPOSAL = PROPOSAL::Components::Component;

namespace corsika::process::proposal {
  using namespace corsika::setup;
  using namespace corsika::environment;
  using namespace corsika::units::si;

  bool Interaction::CanInteract(particles::Code pcode) const noexcept {
    if (std::find(tracked_particles.begin(), tracked_particles.end(), pcode) !=
        tracked_particles.end())
      return true;
    return false;
  }

  template <>
  Interaction::Interaction(SetupEnvironment const& _env, CORSIKA_ParticleCut& _cut)
      : cut(_cut)
      , fRNG(corsika::random::RNGManager::GetInstance().GetRandomStream("proposal")) {
    auto all_compositions = std::vector<const NuclearComposition*>();
    _env.GetUniverse()->walk([&](auto& vtn) {
      if (vtn.HasModelProperties())
        all_compositions.push_back(&vtn.GetModelProperties().GetNuclearComposition());
    });
    for (auto& ncarg : all_compositions) {
      auto comp_vec = std::vector<Component_PROPOSAL>();
      auto frac_iter = ncarg->GetFractions().cbegin();
      for (auto& pcode : ncarg->GetComponents()) {
        comp_vec.emplace_back(GetName(pcode), GetNucleusZ(pcode), GetNucleusA(pcode),
                              *frac_iter);
        ++frac_iter;
      }
      media[ncarg] = PROPOSAL::Medium(
          "Modified Air", 1., PROPOSAL::Air().GetI(), PROPOSAL::Air().GetC(),
          PROPOSAL::Air().GetA(), PROPOSAL::Air().GetM(), PROPOSAL::Air().GetX0(),
          PROPOSAL::Air().GetX1(), PROPOSAL::Air().GetD0(), 1.0, comp_vec);
    }
  }

  template <>
  corsika::process::EProcessReturn Interaction::DoInteraction(
      setup::StackView::StackIterator& vP) {
    if (CanInteract(vP.GetPID())) {
      auto calc = GetCalculator(vP); // [CrossSections]
      std::uniform_real_distribution<double> distr(0., 1.);
      auto [type, comp_ptr, v] =
          get<INTERACTION>(calc->second)
              ->TypeInteraction(vP.GetEnergy() / 1_MeV, distr(fRNG));
      auto rnd =
          vector<double>(get<SECONDARIES>(calc->second)->RequiredRandomNumbers(type));
      for (auto& it : rnd) it = distr(fRNG);
      auto point = PROPOSAL::Vector3D(vP.GetPosition().GetX() / 1_cm,
                                      vP.GetPosition().GetY() / 1_cm,
                                      vP.GetPosition().GetZ() / 1_cm);
      auto d = vP.GetDirection().GetComponents();
      auto direction = PROPOSAL::Vector3D(d.GetX().magnitude(), d.GetY().magnitude(),
                                          d.GetZ().magnitude());
      auto loss = make_tuple(static_cast<int>(type), point, direction,
                             v * vP.GetEnergy() / 1_MeV, 0.);
      auto sec = get<SECONDARIES>(calc->second)
                     ->CalculateSecondaries(vP.GetEnergy() / 1_MeV, loss, *comp_ptr, rnd);
      for (auto& s : sec) {
        auto E = get<PROPOSAL::Loss::ENERGY>(s) * 1_MeV;
        auto vec = corsika::geometry::QuantityVector(
            get<PROPOSAL::Loss::DIRECTION>(s).GetX() * E,
            get<PROPOSAL::Loss::DIRECTION>(s).GetY() * E,
            get<PROPOSAL::Loss::DIRECTION>(s).GetZ() * E);
        auto p = corsika::stack::MomentumVector(
            corsika::geometry::RootCoordinateSystem::GetInstance()
                .GetRootCoordinateSystem(),
            vec);
        auto sec_code = corsika::particles::ConvertFromPDG(
            static_cast<particles::PDGCode>(get<PROPOSAL::Loss::TYPE>(s)));
        vP.AddSecondary(make_tuple(sec_code, E, p, vP.GetPosition(), vP.GetTime()));
      }
    }
    return process::EProcessReturn::eOk;
  }

  template <>
  corsika::units::si::GrammageType Interaction::GetInteractionLength(
      setup::Stack::StackIterator const& vP) {
    if (CanInteract(vP.GetPID())) {
      auto calc = GetCalculator(vP);
      return get<INTERACTION>(calc->second)->MeanFreePath(vP.GetEnergy() / 1_MeV) * 1_g /
             (1_cm * 1_cm);
    }
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }
} // namespace corsika::process::proposal
