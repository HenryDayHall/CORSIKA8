/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/modules/proposal/ProposalProcessBase.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <corsika/setup/SetupEnvironment.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <tuple>

namespace corsika::proposal {

  inline bool ProposalProcessBase::canInteract(Code pcode) const {
    if (std::find(begin(tracked), end(tracked), pcode) != end(tracked)) return true;
    return false;
  }

  inline ProposalProcessBase::ProposalProcessBase(setup::Environment const& _env)
      : RNG_(RNGManager::getInstance().getRandomStream("proposal")) {
    _env.getUniverse()->walk([&](auto& vtn) {
      if (vtn.hasModelProperties()) {
        const auto& prop = vtn.getModelProperties();
        const auto& medium = mediumData(
            prop.getMedium(Point(_env.getCoordinateSystem(), 0_cm, 0_cm, 0_cm)));

        auto comp_vec = std::vector<PROPOSAL::Component>();
        const auto& comp = prop.getNuclearComposition();
        auto frac_iter = comp.getFractions().cbegin();
        for (auto& pcode : comp.getComponents()) {
          comp_vec.emplace_back(std::string(get_name(pcode)), get_nucleus_Z(pcode),
                                get_nucleus_A(pcode), *frac_iter);
          ++frac_iter;
        }

        media[comp.getHash()] = PROPOSAL::Medium(
            medium.getName(), medium.getIeff(), -medium.getCbar(), medium.getAA(),
            medium.getSK(), medium.getX0(), medium.getX1(), medium.getDlt0(),
            medium.getCorrectedDensity(), comp_vec);
      }
    });

    //! If corsika data exist store interpolation tables to the corresponding
    //! path, otherwise interpolation tables would only stored in main memory if
    //! no explicit intrpolation def is specified.
    if (auto data_path = std::getenv("CORSIKA_DATA")) {
      PROPOSAL::InterpolationSettings::TABLES_PATH = std::string(data_path) + "/PROPOSAL";
    } else {
      throw std::runtime_error(
          "It is not recommended to run PROPOSAL without its tables in "
          "$CORSIKA_DATA/PROPOSAL. This would be extremely slow. Please provide the "
          "table directory. ");
    }
  }

  inline size_t ProposalProcessBase::hash::operator()(const calc_key_t& p) const
      noexcept {
    return p.first ^ std::hash<Code>{}(p.second);
  }

} // namespace corsika::proposal
