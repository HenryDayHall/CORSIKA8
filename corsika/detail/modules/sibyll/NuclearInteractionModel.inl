/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>

#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <nuclib.hpp>

namespace corsika::sibyll {

  template <typename TEnvironment, typename TNucleonModel>
  inline NuclearInteractionModel<TEnvironment, TNucleonModel>::NuclearInteractionModel(
      TNucleonModel& hadint, TEnvironment const& env)
      : environment_(env)
      , hadronicInteraction_(hadint) {

    // initialize nuclib
    // TODO: make sure this does not overlap with sibyll
    nuc_nuc_ini_();

    // initialize cross sections
    initializeNuclearCrossSections();
  }

  template <typename TEnvironment, typename TNucleonModel>
  inline NuclearInteractionModel<TEnvironment,
                                 TNucleonModel>::~NuclearInteractionModel() {
    CORSIKA_LOG_DEBUG("Nuclib::NuclearInteractionModel n={} Nnuc={}", count_, nucCount_);
  }

  template <typename TEnvironment, typename TNucleonModel>
  inline void constexpr NuclearInteractionModel<TEnvironment, TNucleonModel>::isValid(
      Code const projectileId, Code const targetId, HEPEnergyType const sqrtSnn,
      unsigned int const projectileA, unsigned int const targetA) const {

    // also depends on underlying model, for Proton/Neutron projectile
    hadronicInteraction_.isValid(Code::Proton, targetId, sqrtSnn, 1, targetA); // throws

    // projectile limits:
    if (is_nucleus(projectileId)) {
      throw std::runtime_error("can only handle nuclear projectile");
    }
    if (projectileA >= getMaxNucleusAProjectile() || projectileA < 2) {
      throw std::runtime_error("projectile mass A out of bounds");
    }
  } // namespace corsika::sibyll

  template <typename TEnvironment, typename TNucleonModel>
  inline void
  NuclearInteractionModel<TEnvironment, TNucleonModel>::printCrossSectionTable(
      Code const pCode) const {
    if (pCode == Code::Argon) {
      CORSIKA_LOG_WARN("SIBYLL cannot handle Argon as target!");
      return;
    }
    int const k = targetComponentsIndex_.at(pCode);
    Code const pNuclei[] = {Code::Helium, Code::Lithium7, Code::Oxygen,
                            Code::Neon,   Code::Argon,    Code::Iron};

    std::ostringstream table;
    table << "Nuclear CrossSectionTable pCode=" << pCode << " :\n en/A ";
    for (auto& j : pNuclei) table << std::setw(9) << j;
    table << "\n";

    // loop over energy bins
    for (unsigned int i = 0; i < getNEnergyBins(); ++i) {
      table << " " << i << "  ";

      for (auto& n : pNuclei) {
        auto const j = get_nucleus_A(n);
        table << " " << std::setprecision(5) << std::setw(8)
              << cnucsignuc_.sigma[j - 1][k][i];
      }
      table << "\n";
    }
    CORSIKA_LOG_DEBUG(table.str());
  }

  template <typename TEnvironment, typename TNucleonModel>
  inline void
  NuclearInteractionModel<TEnvironment, TNucleonModel>::initializeNuclearCrossSections() {

    auto& universe = *(environment_.getUniverse());
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

    CORSIKA_LOG_DEBUG("initializing nuclear cross sections...");

    // loop over target components, at most 4!!
    int k = -1;
    for (Code const ptarg : allElementsInUniverse) {
      if (ptarg == Code::Argon) continue; // NEED TO IGNORE Argon ....
      ++k;
      CORSIKA_LOG_DEBUG("init target component: {}", ptarg);
      int const ib =
          get_nucleus_A(ptarg); // this assumes the universe is only made out of "Nuclei"
      hadronicInteraction_.isValid(Code::Proton, ptarg, 100_GeV, 1, ib); // throws
      targetComponentsIndex_.insert(std::pair<Code, int>(ptarg, k));
      // loop over energies, fNEnBins log. energy bins
      for (unsigned int i = 0; i < getNEnergyBins(); ++i) {
        // hard coded energy grid, has to be aligned to definition in signuc2!!, no
        // comment..
        HEPEnergyType const Ecm = pow(10., 1. + 1. * i) * 1_GeV;
        // get p-p cross sections
        auto const protonId = Code::Proton;
        auto const [siginel, sigela] =
            hadronicInteraction_.getCrossSectionInelEla(protonId, protonId, Ecm);
        const double dsig = siginel / 1_mb;
        const double dsigela = sigela / 1_mb;
        // loop over projectiles, mass numbers from 2 to fMaxNucleusAProjectile
        for (unsigned int j = 1; j < gMaxNucleusAProjectile_; ++j) {
          const int jj = j + 1;
          double sig_out, dsig_out, sigqe_out, dsigqe_out;
          sigma_mc_(jj, ib, dsig, dsigela, gNSample_, sig_out, dsig_out, sigqe_out,
                    dsigqe_out);
          // write to table
          cnucsignuc_.sigma[j][k][i] = sig_out;
          cnucsignuc_.sigqe[j][k][i] = sigqe_out;
        }
      }
    }
    CORSIKA_LOG_DEBUG("cross sections for {} components initialized!",
                      targetComponentsIndex_.size());
    for (auto& ptarg : allElementsInUniverse) { printCrossSectionTable(ptarg); }
  }

  template <typename TEnvironment, typename TNucleonModel>
  inline CrossSectionType
  NuclearInteractionModel<TEnvironment, TNucleonModel>::readCrossSectionTable(
      int const ia, Code const pTarget, HEPEnergyType const elabnuc) const {

    int const ib = targetComponentsIndex_.at(pTarget) + 1; // table index in fortran
    auto const ECoMNuc = sqrt(2. * constants::nucleonMass * elabnuc);
    if (ECoMNuc < getMinEnergyPerNucleonCoM() || ECoMNuc > getMaxEnergyPerNucleonCoM()) {
      throw std::runtime_error("energy outside tabulated range!");
    }
    double const e0 = elabnuc / 1_GeV;
    double sig;
    CORSIKA_LOG_DEBUG("ReadCrossSectionTable: {} {} {}", ia, ib, e0);
    signuc2_(ia, ib, e0, sig);
    CORSIKA_LOG_DEBUG("ReadCrossSectionTable: sig={}", sig);
    return sig * 1_mb;
  }

  // TODO: remove elastic cross section?
  template <typename TEnvironment, typename TNucleonModel>
  CrossSectionType inline NuclearInteractionModel<
      TEnvironment, TNucleonModel>::getCrossSection(Code const projectileId,
                                                    Code const targetId,
                                                    HEPEnergyType const sqrtSnn,
                                                    unsigned int const projectileA,
                                                    unsigned int const targetA) const {

    isValid(projectileId, targetId, sqrtSnn, projectileA, targetA); // throws
    HEPEnergyType const LabEnergyPerNuc =
        static_pow<2>(sqrtSnn) / (2 * constants::nucleonMass);
    auto const sigProd = readCrossSectionTable(projectileA, targetId, LabEnergyPerNuc);
    CORSIKA_LOG_DEBUG("cross section (mb): {}", sigProd / 1_mb);
    return sigProd;
  }

  template <typename TEnvironment, typename TNucleonModel>
  template <typename TSecondaryView>
  inline void NuclearInteractionModel<TEnvironment, TNucleonModel>::doInteraction(
      TSecondaryView& view, COMBoost const& boost, Code const projectileId,
      Code const targetId, HEPEnergyType const sqrtSnn, unsigned int const projectileA,
      unsigned int const targetA) {

    isValid(projectileId, targetId, sqrtSnn, projectileA, targetA); // throws

    CORSIKA_LOG_DEBUG("pId={} tId={} sqrtSnn={}GeV", projectileId, targetId,
                      sqrtSnn / 1_GeV);
    count_++;

    HEPEnergyType const ProjMass = projectileA * constants::nucleonMass;
    // is this really more precise?:
    //        projectile.getNuclearZ() * Proton::mass +
    //      (projectile.getNuclearA() - projectile.getNuclearZ()) * Neutron::mass;

    // lab. Energy per projectile nucleon
    HEPEnergyType const eProjectileLab =
        static_pow<2>(sqrtSnn) / (2 * constants::nucleonMass);
    HEPMomentumType const pProjectileLab =
        sqrt(static_pow<2>(eProjectileLab) - static_pow<2>(constants::nucleonMass));
    MomentumVector const p3ProjectileLab(boost.getRotatedCS(),
                                         {0_GeV, 0_GeV, pProjectileLab});

    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int kATarget = -1;
    if (is_nucleus(targetId)) {
      kATarget = targetA;
    } else if (targetId == Code::Proton) {
      kATarget = 1;
    }
    CORSIKA_LOG_DEBUG("nuclib target code: {}", kATarget);

    // end of target sampling

    // superposition
    CORSIKA_LOG_DEBUG("sampling nuc. multiple interaction structure.. ");
    // get nucleon-nucleon cross section
    // (needed to determine number of nucleon-nucleon scatterings)
    auto const protonId = Code::Proton;
    auto const [prodCrossSection, elaCrossSection] =
        hadronicInteraction_.getCrossSectionInelEla(protonId, protonId, sqrtSnn);
    double const sigProd = prodCrossSection / 1_mb;
    double const sigEla = elaCrossSection / 1_mb;
    // sample number of interactions (only input variables, output in common cnucms)
    // nuclear multiple scattering according to glauber (r.i.p.)
    int_nuc_(kATarget, projectileA, sigProd, sigEla);

    CORSIKA_LOG_DEBUG(
        "number of nucleons in target           : {}\n"
        "number of wounded nucleons in target   : {}\n"
        "number of nucleons in projectile       : {}\n"
        "number of wounded nucleons in project. : {}\n"
        "number of inel. nuc.-nuc. interactions : {}\n"
        "number of elastic nucleons in target   : {}\n"
        "number of elastic nucleons in project. : {}\n"
        "impact parameter: {}",
        kATarget, cnucms_.na, projectileA, cnucms_.nb, cnucms_.ni, cnucms_.nael,
        cnucms_.nbel, cnucms_.b);

    // calculate fragmentation
    CORSIKA_LOG_DEBUG("calculating nuclear fragments..");
    // number of interactions
    // include elastic
    int const nElasticNucleons = cnucms_.nbel;
    int const nInelNucleons = cnucms_.nb;
    int const nIntProj = nInelNucleons + nElasticNucleons;
    double const impactPar = cnucms_.b; // only needed to avoid passing common var.
    int nFragments = 0;
    // number of fragments is limited to 60
    int AFragments[60];
    // call fragmentation routine
    // input: target A, projectile A, number of int. nucleons in projectile, impact
    // parameter (fm) output: nFragments, AFragments in addition the momenta ar stored
    // in pf in common fragments, neglected
    fragm_(kATarget, projectileA, nIntProj, impactPar, nFragments, AFragments);

    // this should not occur but well :)  (LCOV_EXCL_START)
    if (nFragments > (int)getMaxNFragments()) {
      throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");
    }
    // (LCOV_EXCL_STOP)

    // position and time of interaction, not used in NUCLIB
    Point pOrig{boost.getOriginalCS(), {0_m, 0_m, 0_m}}; // = projectile.getPosition();
    TimeType delay = 0_s;                                // there is no time in sibyll

    CORSIKA_LOG_DEBUG("Interaction: position of interaction: {} {}",
                      pOrig.getCoordinates(), delay / 1_s);
    CORSIKA_LOG_DEBUG("number of fragments: {}", nFragments);
    CORSIKA_LOG_DEBUG("adding nuclear fragments to particle stack..");
    // put nuclear fragments on corsika stack
    for (int j = 0; j < nFragments; ++j) {
      CORSIKA_LOG_DEBUG("fragment {}: A={} px={} py={} pz={}", j, AFragments[j],
                        fragments_.ppp[j][0], fragments_.ppp[j][1], fragments_.ppp[j][2]);
      Code specCode;
      auto const nuclA = AFragments[j];
      // get Z from stability line
      auto const nuclZ = int(nuclA / 2.15 + 0.7);

      // TODO: do we need to catch single nucleons??
      if (nuclA == 1) {
        // TODO: sample neutron or proton
        specCode = Code::Proton;
      } else {
        specCode = Code::Nucleus;
      }
      HEPMassType const mass = get_nucleus_mass(nuclA, nuclZ);

      CORSIKA_LOG_DEBUG("adding fragment: {}", get_name(specCode));
      CORSIKA_LOG_DEBUG("A,Z: {}, {}", nuclA, nuclZ);
      CORSIKA_LOG_DEBUG("mass: {} GeV", mass / 1_GeV);

      // CORSIKA 7 way
      // spectators inherit momentum from original projectile
      double const mass_ratio = mass / ProjMass;
      auto const p3lab = p3ProjectileLab * mass_ratio;

      CORSIKA_LOG_DEBUG("mass ratio {}, fragment momentum {}", mass_ratio,
                        p3lab.getComponents() / 1_GeV);

      if (nuclA == 1) {
        // add nucleon
        view.addSecondary(std::make_tuple(specCode, p3lab, pOrig, delay));
      } else {
        // add nucleus
        view.addSecondary(std::make_tuple(specCode, p3lab, pOrig, delay, nuclA, nuclZ));
      }
    }

    // add elastic nucleons to corsika stack
    // TODO: the elastic interaction could be external like the inelastic interaction,
    // e.g. use existing ElasticModel
    CORSIKA_LOG_DEBUG("adding elastically scattered nucleons to particle stack..");
    for (int j = 0; j < nElasticNucleons; ++j) {
      // TODO: sample proton or neutron
      Code const elaNucCode = Code::Proton;

      // CORSIKA 7 way
      // elastic nucleons inherit momentum from original projectile
      // neglecting momentum transfer in interaction
      double const mass_ratio = get_mass(elaNucCode) / ProjMass;
      auto const p3lab = p3ProjectileLab * mass_ratio;
      view.addSecondary(std::make_tuple(elaNucCode, p3lab, pOrig, delay));
    }

    // add inelastic interactions
    CORSIKA_LOG_DEBUG("calculate inelastic nucleon-nucleon interactions..");
    for (int j = 0; j < nInelNucleons; ++j) {
      // TODO: sample neutron or proton
      auto pCode = Code::Proton;
      // temporarily add to stack, will be removed after interaction in DoInteraction
      CORSIKA_LOG_DEBUG("inelastic interaction no. {}", j);
      typename TSecondaryView::inner_stack_value_type nucleonStack;
      auto inelasticNucleon =
          nucleonStack.addParticle(std::make_tuple(pCode, p3ProjectileLab, pOrig, delay));
      inelasticNucleon.setNode(view.getProjectile().getNode());
      // create inelastic interaction for each nucleon
      CORSIKA_LOG_TRACE("calling HadronicInteraction...");
      // create new StackView for each of the nucleons
      TSecondaryView nucleon_secondaries(inelasticNucleon);
      // all inner hadronic event generator
      hadronicInteraction_.doInteraction(nucleon_secondaries, boost, pCode, targetId,
                                         sqrtSnn, 0, targetA);
      for (const auto& pSec : nucleon_secondaries) {
        view.addSecondary(std::make_tuple(pSec.getPID(), pSec.getMomentum(),
                                          pSec.getPosition(), pSec.getTime()));
      }
    }
  }

} // namespace corsika::sibyll
