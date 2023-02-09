/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <algorithm>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <set>
#include <utility>

#include <boost/iterator/zip_iterator.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>

#include <corsika/framework/core/PhysicalUnits.hpp>

#include <FLUKA.hpp>
#include <corsika/modules/fluka/ParticleConversion.hpp>

namespace corsika::fluka {
  template <typename TEnvironment>
  inline InteractionModel::InteractionModel(TEnvironment const& env)
      : materials_{genFlukaMaterials(env)} {
    for (auto const& [code, matno] : materials_) {
      std::cout << get_name(code, full_name{}) << "    " << matno << std::endl;
    }
  }

  inline bool InteractionModel::isValid(Code projectileID, int material,
                                        HEPEnergyType /*sqrtS*/) const {
    if (!fluka::canInteract(projectileID)) {
      // invalid projectile
      //~ std::cout << "invalid projecile (cannot interact)";
      return false;
    }

    if (material < 0) {
      // invalid/uninitialized target
      return false;
    }

    // TODO: check validity range of sqrtS
    return true;
  }

  inline bool InteractionModel::isValid(Code projectileID, Code targetID,
                                        HEPEnergyType sqrtS) const {
    return isValid(projectileID, getMaterialIndex(targetID), sqrtS);
  }

  inline int InteractionModel::getMaterialIndex(Code targetID) const {
    auto compare = [=](std::pair<Code, int> const& p) { return p.first == targetID; };
    if (auto it = std::find_if(materials_.cbegin(), materials_.cend(), compare);
        it == materials_.cend()) {
      return -1;
    } else {
      return it->second;
    }
  }

  inline CrossSectionType InteractionModel::getCrossSection(
      Code const projectileId, Code const targetId, FourMomentum const& projectileP4,
      FourMomentum const& targetP4) const {
    auto const flukaCodeProj =
        static_cast<FLUKACodeIntType>(convertToFluka(projectileId));
    auto const flukaMaterial = getMaterialIndex(targetId);

    HEPEnergyType const sqrtS = (projectileP4 + targetP4).getNorm();
    if (!isValid(projectileId, flukaMaterial, sqrtS)) { return CrossSectionType::zero(); }

    COMBoost const targetRestBoost{targetP4.getSpaceLikeComponents(), get_mass(targetId)};
    FourMomentum const projectileLab4mom = targetRestBoost.toCoM(projectileP4);
    HEPEnergyType const Elab = projectileLab4mom.getTimeLikeComponent();
    auto constexpr invGeV = 1 / 1_GeV;
    double const EkinLab = (Elab - get_mass(projectileId)) * invGeV;
    double const labMomentum =
        projectileLab4mom.getSpaceLikeComponents().getNorm() * invGeV;

    auto const plab = projectileLab4mom.getSpaceLikeComponents();

    std::cout << targetRestBoost.boost_ << '\n';
    std::cout << targetRestBoost.rotatedCS_->getTransform().matrix() << '\n';
    std::cout << "Elab / GeV = " << Elab * invGeV << '\n';
    std::cout << "plab = " << plab << '\n';
    std::cout << "EkinLab / GeV = " << EkinLab << '\n';
    std::cout << "labMomentum / GeV = "
              << projectileLab4mom.getSpaceLikeComponents() * invGeV << '\n';

    CrossSectionType const xs = ::fluka::sgmxyz_(&flukaCodeProj, &flukaMaterial, &EkinLab,
                                                 &labMomentum, &iflxyz_) *
                                1_mb;
    return xs;
  }

  template <typename TSecondaryView>
  inline void InteractionModel::doInteraction(TSecondaryView& view,
                                              Code const projectileId,
                                              Code const targetId,
                                              FourMomentum const& projectileP4,
                                              FourMomentum const& targetP4) {}

  template <typename TEnvironment>
  inline std::vector<std::pair<Code, int>> InteractionModel::genFlukaMaterials(
      TEnvironment const& env) {
    auto const& universe = *(env.getUniverse());
    // generate complete list of all nuclei types in universe

    auto const allElementsInUniverse = std::invoke([&]() {
      std::set<Code> allElementsInUniverse;
      auto collectElements = [&](auto& vtn) {
        if (vtn.hasModelProperties()) {
          auto const& comp =
              vtn.getModelProperties().getNuclearComposition().getComponents();
          for (auto const c : comp) allElementsInUniverse.insert(c);
        }
      };
      universe.walk(collectElements);
      return allElementsInUniverse;
    });

    /*
     * We define one material per element/isotope we have in C8. Cross-section averaging
     * and target isotope selection happen in C8.
     */

    int const nElements = allElementsInUniverse.size();
    auto nelmfl = std::make_unique<int[]>(nElements);
    std::vector<int> izelfl;
    izelfl.reserve(nElements);
    auto wfelml = std::make_unique<double[]>(nElements);
    auto const mxelfl = nElements;
    double const pptmax = 1e11; // GeV
    double const ef2dp3 = 0;    // GeV, 0 means default is used
    double const df2dp3 = -1;   // default
    bool const lprint = true;
    auto mtflka = std::make_unique<int[]>(mxelfl);
    char crvrck[8 + 1] =
        "76466879"; // magic number that FLUKA uses to see if it's the right version

    /*
     *    Iflxyz =  1 -> only inelastic
     *    Iflxyz = 10 -> only elastic
     *    Iflxyz = 11 -> inelastic + elastic
     *    Iflxyz =100 -> only emd
     *    Iflxyz =101 -> inelastic + emd
     *    Iflxyz =110 -> elastic + emd
     *    Iflxyz =111 -> inelastic + elastic + emd
     */

    std::fill(&nelmfl[0], &nelmfl[nElements], 1);
    std::fill(&wfelml[0], &wfelml[nElements], 1.);
    std::transform(allElementsInUniverse.cbegin(), allElementsInUniverse.cend(),
                   std::back_inserter(izelfl), get_nucleus_Z);

    // check if FLUPRO is set
    if (std::getenv("FLUPRO") == nullptr) {
      throw std::runtime_error{"FLUPRO not set; required to initialize FLUKA"};
    }

    // call FLUKA
    ::fluka::stpxyz_(&nElements, nelmfl.get(), izelfl.data(), wfelml.get(), &mxelfl,
                     &pptmax, &ef2dp3, &df2dp3, &iflxyz_, &lprint, mtflka.get(), crvrck);

    // now create & fill vector of (C8 Code, FLUKA mat. no.) pairs
    std::vector<std::pair<Code, int>> mapping;
    mapping.reserve(nElements);

    auto it = boost::make_zip_iterator(
        boost::make_tuple(allElementsInUniverse.begin(), &mtflka[0]));
    auto const end = boost::make_zip_iterator(
        boost::make_tuple(allElementsInUniverse.end(), &mtflka[nElements]));
    for (; it != end; ++it) {
      boost::tuple<Code const&, int&> tup = *it;
      mapping.emplace_back(tup.get<0>(), tup.get<1>());
    }

    return mapping;
  }
} // namespace corsika::fluka
