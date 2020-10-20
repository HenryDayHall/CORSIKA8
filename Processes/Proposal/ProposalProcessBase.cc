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
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::process::proposal {
  bool ProposalProcessBase::CanInteract(particles::Code pcode) const {
    if (std::find(begin(tracked), end(tracked), pcode) != end(tracked)) return true;
    return false;
  }

  ProposalProcessBase::ProposalProcessBase(setup::Environment const& _env,
                                           corsika::units::si::HEPEnergyType _emCut)
      : emCut_(_emCut)
      , fRNG(corsika::random::RNGManager::GetInstance().GetRandomStream("proposal")) {
    using namespace corsika::units::si; // required for operator::_MeV
    _env.GetUniverse()->walk([&](auto& vtn) {
      if (vtn.HasModelProperties()) {
        auto prop = &vtn.GetModelProperties();
        auto medium = mediumData(prop->medium(corsika::geometry::Point(
            geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem(), 0_cm,
            0_cm, 0_cm)));

        auto comp_vec = std::vector<PROPOSAL::Components::Component>();
        auto comp = prop->GetNuclearComposition();
        auto frac_iter = comp.GetFractions().cbegin();
        for (auto& pcode : comp.GetComponents()) {
          comp_vec.emplace_back(GetName(pcode), GetNucleusZ(pcode), GetNucleusA(pcode),
                                *frac_iter);
          ++frac_iter;
        }

        media[comp.hash()] =
            PROPOSAL::Medium(medium.name(), medium.Ieff(), -medium.Cbar(), medium.aa(), medium.sk(),
                             medium.x0(), medium.x1(), medium.dlt0(), medium.corrected_density(), comp_vec);
      }
    });

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
    return p.first ^ std::hash<particles::Code>{}(p.second);
  }

} // namespace corsika::process::proposal
