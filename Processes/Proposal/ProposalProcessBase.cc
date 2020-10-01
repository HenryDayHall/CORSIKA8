/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/process/proposal/ProposalProcessBase.h>
#include <corsika/setup/SetupEnvironment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>
#include <cstdlib>
#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::process::proposal {
  bool ProposalProcessBase::CanInteract(particles::Code pcode) const {
    if (std::find(begin(tracked), end(tracked), pcode) != end(tracked)) return true;
    return false;
  }

  ProposalProcessBase::ProposalProcessBase(setup::SetupEnvironment const& _env,
                                           particle_cut::ParticleCut& _cut)
      : cut(_cut)
      , fRNG(corsika::random::RNGManager::GetInstance().GetRandomStream("proposal")) {
    auto all_compositions = std::vector<const environment::NuclearComposition*>();
    _env.GetUniverse()->walk([&](auto& vtn) {
      if (vtn.HasModelProperties())
        all_compositions.push_back(&vtn.GetModelProperties().GetNuclearComposition());
    });
    for (auto& ncarg : all_compositions) {
      auto comp_vec = std::vector<PROPOSAL::Components::Component>();
      auto frac_iter = ncarg->GetFractions().cbegin();
      for (auto& pcode : ncarg->GetComponents()) {
        comp_vec.emplace_back(GetName(pcode), GetNucleusZ(pcode), GetNucleusA(pcode),
                              *frac_iter);
        ++frac_iter;
      }
      media[ncarg] = PROPOSAL::Medium(
          "Modified Air", PROPOSAL::Air().GetI(), PROPOSAL::Air().GetC(),
          PROPOSAL::Air().GetA(), PROPOSAL::Air().GetM(), PROPOSAL::Air().GetX0(),
          PROPOSAL::Air().GetX1(), PROPOSAL::Air().GetD0(), 1.0, comp_vec);
    }
    PROPOSAL::InterpolationDef::order_of_interpolation = 2;
    PROPOSAL::InterpolationDef::nodes_cross_section = 100;
    PROPOSAL::InterpolationDef::nodes_propagate = 1000;

    //! If corsika data exist store interpolation tables to the corresponding
    //! path, otherwise interpolation tables would only stored in main memory if
    //! no explicit intrpolation def is specified.
    if (auto data_path = std::getenv("CORSIKA_DATA")) {
      PROPOSAL::InterpolationDef::path_to_tables = std::string(data_path) + "/PROPOSAL";
    } else {
      throw std::runtime_error(
          "It is not recommended to run PROPOSAL without its tables in "
          "$CORSIKA_DATA/PROPOSAL. This would be extremely slow. Please provide the "
          "table directory. ");
    }
  }

  size_t ProposalProcessBase::hash::operator()(const calc_key_t& p) const noexcept {
    return std::hash<const environment::NuclearComposition*>{}(p.first) ^
           std::hash<particles::Code>{}(p.second);
  }

} // namespace corsika::process::proposal
