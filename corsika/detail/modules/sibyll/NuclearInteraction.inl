/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Interaction.hpp>
#include <corsika/modules/sibyll/NuclearInteraction.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <nuclib.hpp>

namespace corsika::sibyll {

  template <typename TEnvironment>
  inline NuclearInteraction<TEnvironment>::NuclearInteraction(sibyll::Interaction& hadint,
                                                              TEnvironment const& env)
      : environment_(env)
      , hadronicInteraction_(hadint) {

    // initialize hadronic interaction module

    // check compatibility of energy ranges, someone could try to use low-energy model..
    if (!hadronicInteraction_.isValidCoMEnergy(getMinEnergyPerNucleonCoM()) ||
        !hadronicInteraction_.isValidCoMEnergy(getMaxEnergyPerNucleonCoM())) {
      throw std::runtime_error(
          "NuclearInteraction: hadronic interaction model incompatible!");
    }

    // initialize nuclib
    // TODO: make sure this does not overlap with sibyll
    nuc_nuc_ini_();

    // initialize cross sections
    initializeNuclearCrossSections();
  }

  template <typename TEnvironment>
  inline NuclearInteraction<TEnvironment>::~NuclearInteraction() {
    CORSIKA_LOG_DEBUG("Nuclib::NuclearInteraction n={} Nnuc={}", count_, nucCount_);
  }

  template <typename TEnvironment>
  inline void NuclearInteraction<TEnvironment>::printCrossSectionTable(Code pCode) {
    if (pCode == Code::Argon) {
      CORSIKA_LOG_WARN("SIBYLL cannot handle Argon as target!");
      return;
    }
    const int k = targetComponentsIndex_.at(pCode);
    Code pNuclei[] = {Code::Helium, Code::Lithium7, Code::Oxygen,
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

  template <typename TEnvironment>
  inline void NuclearInteraction<TEnvironment>::initializeNuclearCrossSections() {

    auto& universe = *(environment_.getUniverse());

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

    CORSIKA_LOG_DEBUG("NuclearInteraction: initializing nuclear cross sections...");

    // loop over target components, at most 4!!
    int k = -1;
    for (auto& ptarg : allElementsInUniverse) {
      if (ptarg == Code::Argon) continue; // NEED TO IGNORE Argon ....
      ++k;
      CORSIKA_LOG_DEBUG("NuclearInteraction: init target component: {}", ptarg);
      const int ib = get_nucleus_A(ptarg);
      if (!hadronicInteraction_.isValidTarget(ptarg)) {
        CORSIKA_LOG_DEBUG(
            "NuclearInteraction::InitializeNuclearCrossSections: target nucleus? id={}",
            ptarg);
        throw std::runtime_error(
            " target can not be handled by hadronic interaction model! ");
      }
      targetComponentsIndex_.insert(std::pair<Code, int>(ptarg, k));
      // loop over energies, fNEnBins log. energy bins
      for (unsigned int i = 0; i < getNEnergyBins(); ++i) {
        // hard coded energy grid, has to be aligned to definition in signuc2!!, no
        // comment..
        const HEPEnergyType Ecm = pow(10., 1. + 1. * i) * 1_GeV;
        // get p-p cross sections
        auto const protonId = Code::Proton;
        auto const [siginel, sigela] =
            hadronicInteraction_.getCrossSection(protonId, protonId, Ecm);
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
    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: cross sections for {} "
        " components initialized!",
        targetComponentsIndex_.size());
    for (auto& ptarg : allElementsInUniverse) { printCrossSectionTable(ptarg); }
  }

  template <typename TEnvironment>
  inline CrossSectionType NuclearInteraction<TEnvironment>::readCrossSectionTable(
      const int ia, Code pTarget, HEPEnergyType elabnuc) {

    const int ib = targetComponentsIndex_.at(pTarget) + 1; // table index in fortran
    auto const ECoMNuc = sqrt(2. * constants::nucleonMass * elabnuc);
    if (ECoMNuc < getMinEnergyPerNucleonCoM() || ECoMNuc > getMaxEnergyPerNucleonCoM())
      throw std::runtime_error("NuclearInteraction: energy outside tabulated range!");
    const double e0 = elabnuc / 1_GeV;
    double sig;
    CORSIKA_LOG_DEBUG("ReadCrossSectionTable: {} {} {}", ia, ib, e0);
    signuc2_(ia, ib, e0, sig);
    CORSIKA_LOG_DEBUG("ReadCrossSectionTable: sig={}", sig);
    return sig * 1_mb;
  }

  // TODO: remove elastic cross section?
  template <typename TEnvironment>
  template <typename TParticle>
  std::tuple<CrossSectionType, CrossSectionType> inline NuclearInteraction<
      TEnvironment>::getCrossSection(TParticle const& projectile, Code const TargetId) {

    if (!is_nucleus(projectile.getPID())) {
      throw std::runtime_error(
          "NuclearInteraction: getCrossSection: particle not a nucleus!");
    }

    unsigned int const iBeamA = get_nucleus_A(projectile.getPID());
    HEPEnergyType LabEnergyPerNuc = projectile.getEnergy() / iBeamA;
    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: getCrossSection: called with: beamNuclA={} "
        " TargetId={} LabEnergyPerNuc={}GeV ",
        iBeamA, TargetId, LabEnergyPerNuc / 1_GeV);

    // use nuclib to calc. nuclear cross sections
    // TODO: for now assumes air with hard coded composition
    // extend to arbitrary mixtures, requires smarter initialization
    // get nuclib projectile code: nucleon number
    if (iBeamA > getMaxNucleusAProjectile() || iBeamA < 2) {
      CORSIKA_LOG_DEBUG(
          "NuclearInteraction: beam nucleus outside allowed range for NUCLIB!"
          "A=" +
          std::to_string(iBeamA));
      throw std::runtime_error(
          "NuclearInteraction: getCrossSection: beam nucleus outside allowed range for "
          "NUCLIB!");
    }

    if (hadronicInteraction_.isValidTarget(TargetId)) {
      auto const sigProd = readCrossSectionTable(iBeamA, TargetId, LabEnergyPerNuc);
      CORSIKA_LOG_DEBUG("cross section (mb): " + std::to_string(sigProd / 1_mb));
      return std::make_tuple(sigProd, 0_mb);
    } else {
      throw std::runtime_error("target outside range.");
    }
    return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mb,
                           std::numeric_limits<double>::infinity() * 1_mb);
  }

  template <typename TEnvironment>
  template <typename TParticle>
  inline GrammageType NuclearInteraction<TEnvironment>::getInteractionLength(
      TParticle const& projectile) {

    // coordinate system, get global frame of reference

    const Code corsikaBeamId = projectile.getPID();

    if (!is_nucleus(corsikaBeamId)) {
      // no nuclear interaction
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }

    // read from cross section code table

    MomentumVector pLab = projectile.getMomentum();
    CoordinateSystemPtr const& labCS = pLab.getCoordinateSystem();

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(labCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = projectile.getEnergy() + constants::nucleonMass;
    int const nuclA = get_nucleus_A(corsikaBeamId);
    auto const ElabNuc = projectile.getEnergy() / nuclA;

    MomentumVector pTotLab(labCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    pTotLab += pLab;
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.getNorm();
    // calculate cm. energy
    [[maybe_unused]] HEPEnergyType const ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy
    auto const ECoMNN = sqrt(2. * ElabNuc * constants::nucleonMass);
    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: LambdaInt: \n"
        " input energy: {}GeV\n"
        " input energy CoM: {}GeV\n"
        " beam pid: {}\n"
        " beam A: {}\n"
        " input energy per nucleon: {}GeV\n"
        " input energy CoM per nucleon: {}GeV ",
        Elab / 1_GeV, ECoM / 1_GeV, get_name(corsikaBeamId), nuclA, ElabNuc / 1_GeV,
        ECoMNN / 1_GeV);

    // energy limits
    // TODO: values depend on hadronic interaction model !! this is sibyll specific
    if (ElabNuc >= 8.5_GeV && ECoMNN >= gMinEnergyPerNucleonCoM_ &&
        ECoMNN < gMaxEnergyPerNucleonCoM_) {

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */
      auto const* const currentNode = projectile.getNode();
      auto const& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();
      // determine average interaction length
      // weighted sum
      int i = -1;
      CrossSectionType weightedProdCrossSection = 0_mb;
      // get weights of components from environment/medium
      const auto& w = mediumComposition.getFractions();
      // loop over components in medium
      for (auto const targetId : mediumComposition.getComponents()) {
        if (targetId == Code::Argon) continue; // NEED TO IGNORE Argon ....
        i++;
        CORSIKA_LOG_DEBUG("NuclearInteraction: get interaction length for target: {}",
                          get_name(targetId));
        auto const [productionCrossSection, elaCrossSection] =
            getCrossSection(projectile, targetId);
        [[maybe_unused]] auto& dummy_elaCrossSection = elaCrossSection;

        CORSIKA_LOG_DEBUG(
            "NuclearInteraction: "
            "IntLength: nuclib return (mb): " +
            std::to_string(productionCrossSection / 1_mb));
        weightedProdCrossSection += w[i] * productionCrossSection;
      }
      CORSIKA_LOG_DEBUG(
          "NuclearInteraction: "
          "IntLength: weighted CrossSection (mb): {} ",
          weightedProdCrossSection / 1_mb);

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      CORSIKA_LOG_DEBUG(
          "NuclearInteraction: "
          "interaction length (g/cm2): {} ",
          int_length * (1_cm * 1_cm / (0.001_kg)));

      return int_length;
    } else {
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }
  }

  template <typename TEnvironment>
  template <typename TSecondaryView>
  inline void NuclearInteraction<TEnvironment>::doInteraction(TSecondaryView& view) {

    auto projectile = view.getProjectile();

    // this routine superimposes different nucleon-nucleon interactions
    // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC

    const auto ProjId = projectile.getPID();
    // TODO: calculate projectile mass in nuclearStackExtension
    //      const auto ProjMass = projectile.getMass();

    CORSIKA_LOG_DEBUG("NuclearInteraction: DoInteraction: called with: {}",
                      get_name(ProjId));

    // check if target-style nucleus (enum)
    if (!is_nucleus(ProjId)) {
      throw std::runtime_error(
          "NuclearInteraction: DoInteraction: Wrong nucleus type. Nuclear projectiles "
          "should use NuclearStackExtension!");
    }

    auto const ProjMass = get_mass(ProjId);
    CORSIKA_LOG_DEBUG("NuclearInteraction: projectile mass: {} ", ProjMass / 1_GeV);

    count_++;

    // position and time of interaction, not used in NUCLIB
    Point pOrig = projectile.getPosition();
    TimeType tOrig = projectile.getTime();

    CORSIKA_LOG_DEBUG("Interaction: position of interaction: {}", pOrig.getCoordinates());
    CORSIKA_LOG_DEBUG("Interaction: time: {} ", tOrig / 1_s);

    // projectile nucleon number
    const unsigned int kAProj = get_nucleus_A(ProjId);
    if (kAProj > getMaxNucleusAProjectile())
      throw std::runtime_error("Projectile nucleus too large for NUCLIB!");

    // kinematics
    // define projectile nucleus
    HEPEnergyType const eProjectileLab = projectile.getEnergy();
    MomentumVector const pProjectileLab = projectile.getMomentum();
    FourVector const PprojLab(eProjectileLab, pProjectileLab);
    CoordinateSystemPtr const& labCS = pProjectileLab.getCoordinateSystem();

    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: eProj lab: {} "
        "pProj lab: {} ",
        eProjectileLab / 1_GeV, pProjectileLab.getComponents() / 1_GeV);

    // define projectile nucleon
    HEPEnergyType const eProjectileNucLab = eProjectileLab / kAProj;
    MomentumVector const pProjectileNucLab = pProjectileLab / kAProj;
    FourVector const PprojNucLab(eProjectileNucLab, pProjectileNucLab);

    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: eProjNucleon lab (GeV): {} "
        "pProjNucleon lab (GeV): {} ",
        eProjectileNucLab / 1_GeV, pProjectileNucLab.getComponents() / 1_GeV);

    // define target
    // always a nucleon
    // target is always at rest
    auto const eTargetNucLab = 0_GeV + constants::nucleonMass;
    auto const pTargetNucLab = MomentumVector(labCS, 0_GeV, 0_GeV, 0_GeV);
    FourVector const PtargNucLab(eTargetNucLab, pTargetNucLab);

    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: etarget lab(GeV): {} "
        "NuclearInteraction: ptarget lab(GeV): {} ",
        eTargetNucLab / 1_GeV, pTargetNucLab.getComponents() / 1_GeV);

    // center-of-mass energy in nucleon-nucleon frame
    auto const PtotNN4 = PtargNucLab + PprojNucLab;
    HEPEnergyType EcmNN = PtotNN4.getNorm();

    CORSIKA_LOG_DEBUG("NuclearInteraction: nuc-nuc cm energy: {}", EcmNN / 1_GeV);

    if (!hadronicInteraction_.isValidCoMEnergy(EcmNN)) {
      CORSIKA_LOG_DEBUG(
          "NuclearInteraction: nuc-nuc. CoM energy too low for hadronic "
          "interaction model!");
      throw std::runtime_error("NuclearInteraction: DoInteraction: energy too low!");
    }

    // define boost to NUCLEON-NUCLEON frame
    COMBoost const boost(PprojNucLab, constants::nucleonMass);
    // boost projecticle
    auto const PprojNucCoM = boost.toCoM(PprojNucLab);

    // boost target
    auto const PtargNucCoM = boost.toCoM(PtargNucLab);

    CORSIKA_LOG_DEBUG(
        "Interaction: ebeam CoM: {} "
        ", pbeam CoM: {} ",
        PprojNucCoM.getTimeLikeComponent() / 1_GeV,
        PprojNucCoM.getSpaceLikeComponents().getComponents() / 1_GeV);
    CORSIKA_LOG_DEBUG(
        "Interaction: etarget CoM: {}"
        ", ptarget CoM: {}",
        PtargNucCoM.getTimeLikeComponent() / 1_GeV,
        PtargNucCoM.getSpaceLikeComponents().getComponents() / 1_GeV);

    // sample target nucleon number
    //
    // proton stand-in for nucleon
    const auto beamId = Code::Proton;
    auto const* const currentNode = projectile.getNode();
    const auto& mediumComposition =
        currentNode->getModelProperties().getNuclearComposition();
    CORSIKA_LOG_DEBUG("get nucleon-nucleus cross sections for target materials..");
    // get cross sections for target materials
    // using nucleon-target-nucleus cross section!!!
    /*
      Here we read the cross section from the interaction model again,
      should be passed from getInteractionLength if possible
    */
    auto const& compVec = mediumComposition.getComponents();
    std::vector<CrossSectionType> cross_section_of_components(compVec.size());

    for (size_t i = 0; i < compVec.size(); ++i) {
      auto const targetId = compVec[i];
      if (targetId == Code::Argon) continue; // NEED TO IGNORE Argon ....
      CORSIKA_LOG_DEBUG("target component: {}", get_name(targetId));
      CORSIKA_LOG_DEBUG("beam id: {}", get_name(beamId));
      const auto [sigProd, sigEla] =
          hadronicInteraction_.getCrossSection(beamId, targetId, EcmNN);
      cross_section_of_components[i] = sigProd;
      [[maybe_unused]] auto sigElaCopy = sigEla; // ONLY TO AVOID COMPILER WARNINGS
    }

    const auto targetCode =
        mediumComposition.sampleTarget(cross_section_of_components, RNG_);
    CORSIKA_LOG_DEBUG("Interaction: target selected: {}", get_name(targetCode));
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int kATarget = -1;
    if (is_nucleus(targetCode))
      kATarget = get_nucleus_A(targetCode);
    else if (targetCode == Code::Proton)
      kATarget = 1;
    CORSIKA_LOG_DEBUG("NuclearInteraction: nuclib target code: " +
                      std::to_string(kATarget));
    if (!hadronicInteraction_.isValidTarget(targetCode))
      throw std::runtime_error("target outside range. ");
    // end of target sampling

    // superposition
    CORSIKA_LOG_DEBUG(
        "NuclearInteraction: sampling nuc. multiple interaction structure.. ");
    // get nucleon-nucleon cross section
    // (needed to determine number of nucleon-nucleon scatterings)
    const auto protonId = Code::Proton;
    const auto [prodCrossSection, elaCrossSection] =
        hadronicInteraction_.getCrossSection(protonId, protonId, EcmNN);
    const double sigProd = prodCrossSection / 1_mb;
    const double sigEla = elaCrossSection / 1_mb;
    // sample number of interactions (only input variables, output in common cnucms)
    // nuclear multiple scattering according to glauber (r.i.p.)
    int_nuc_(kATarget, kAProj, sigProd, sigEla);

    CORSIKA_LOG_DEBUG(
        "number of nucleons in target           : {}\n"
        "number of wounded nucleons in target   : {}\n"
        "number of nucleons in projectile       : {}\n"
        "number of wounded nucleons in project. : {}\n"
        "number of inel. nuc.-nuc. interactions : {}\n"
        "number of elastic nucleons in target   : {}\n"
        "number of elastic nucleons in project. : {}\n"
        "impact parameter: {}",
        kATarget, cnucms_.na, kAProj, cnucms_.nb, cnucms_.ni, cnucms_.nael, cnucms_.nbel,
        cnucms_.b);

    // calculate fragmentation
    CORSIKA_LOG_DEBUG("calculating nuclear fragments..");
    // number of interactions
    // include elastic
    const int nElasticNucleons = cnucms_.nbel;
    const int nInelNucleons = cnucms_.nb;
    const int nIntProj = nInelNucleons + nElasticNucleons;
    const double impactPar = cnucms_.b; // only needed to avoid passing common var.
    int nFragments = 0;
    // number of fragments is limited to 60
    int AFragments[60];
    // call fragmentation routine
    // input: target A, projectile A, number of int. nucleons in projectile, impact
    // parameter (fm) output: nFragments, AFragments in addition the momenta ar stored
    // in pf in common fragments, neglected
    fragm_(kATarget, kAProj, nIntProj, impactPar, nFragments, AFragments);

    // this should not occur but well :)  (LCOV_EXCL_START)
    if (nFragments > (int)getMaxNFragments())
      throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");
    // (LCOV_EXCL_STOP)

    CORSIKA_LOG_DEBUG("number of fragments: " + std::to_string(nFragments));
    CORSIKA_LOG_DEBUG("adding nuclear fragments to particle stack..");
    // put nuclear fragments on corsika stack
    for (int j = 0; j < nFragments; ++j) {
      CORSIKA_LOG_DEBUG("fragment {}: A={} px={} py={} pz={}", j, AFragments[j],
                        fragments_.ppp[j][0], fragments_.ppp[j][1], fragments_.ppp[j][2]);
      const auto nuclA = AFragments[j];
      // get Z from stability line
      const auto nuclZ = int(nuclA / 2.15 + 0.7);

      // TODO: do we need to catch single nucleons??
      Code specCode = Code::Neutron; //  sample neutron or proton ?
      if (nuclA > 1) specCode = get_nucleus_code(nuclA, nuclZ);
      HEPMassType const mass = get_mass(specCode);

      CORSIKA_LOG_DEBUG("NuclearInteraction: adding fragment: {}", get_name(specCode));
      CORSIKA_LOG_DEBUG("NuclearInteraction: A,Z: {}, {}", nuclA, nuclZ);
      CORSIKA_LOG_DEBUG("NuclearInteraction: mass: {} GeV", std::to_string(mass / 1_GeV));

      // CORSIKA 7 way
      // spectators inherit momentum from original projectile
      const double mass_ratio = mass / ProjMass;

      CORSIKA_LOG_DEBUG("NuclearInteraction: mass ratio " + std::to_string(mass_ratio));

      auto const Plab = PprojLab * mass_ratio;

      CORSIKA_LOG_DEBUG("NuclearInteraction: fragment momentum: {}",
                        Plab.getSpaceLikeComponents().getComponents() / 1_GeV);

      projectile.addSecondary(
          std::make_tuple(specCode, Plab.getSpaceLikeComponents(), pOrig, tOrig));
    }

    // add elastic nucleons to corsika stack
    // TODO: the elastic interaction could be external like the inelastic interaction,
    // e.g. use existing ElasticModel
    CORSIKA_LOG_DEBUG("adding elastically scattered nucleons to particle stack..");
    for (int j = 0; j < nElasticNucleons; ++j) {
      // TODO: sample proton or neutron
      auto const elaNucCode = Code::Proton;

      // CORSIKA 7 way
      // elastic nucleons inherit momentum from original projectile
      // neglecting momentum transfer in interaction
      const double mass_ratio = get_mass(elaNucCode) / ProjMass;
      auto const Plab = PprojLab * mass_ratio;

      projectile.addSecondary(
          std::make_tuple(elaNucCode, Plab.getSpaceLikeComponents(), pOrig, tOrig));
    }

    // add inelastic interactions
    CORSIKA_LOG_DEBUG("calculate inelastic nucleon-nucleon interactions..");
    for (int j = 0; j < nInelNucleons; ++j) {
      // TODO: sample neutron or proton
      auto pCode = Code::Proton;
      // temporarily add to stack, will be removed after interaction in DoInteraction
      CORSIKA_LOG_DEBUG("inelastic interaction no. {}", j);
      typename TSecondaryView::inner_stack_value_type nucleonStack;
      auto inelasticNucleon = nucleonStack.addParticle(
          std::make_tuple(pCode, PprojNucLab.getSpaceLikeComponents(), pOrig, tOrig));
      inelasticNucleon.setNode(projectile.getNode());
      // create inelastic interaction for each nucleon
      CORSIKA_LOG_TRACE("calling HadronicInteraction...");
      // create new StackView for each of the nucleons
      TSecondaryView nucleon_secondaries(inelasticNucleon);
      // all inner hadronic event generator
      hadronicInteraction_.doInteraction(nucleon_secondaries);
      for (const auto& pSec : nucleon_secondaries) {
        projectile.addSecondary(std::make_tuple(pSec.getPID(), pSec.getMomentum(),
                                                pSec.getPosition(), pSec.getTime()));
      }
    }

    CORSIKA_LOG_DEBUG("NuclearInteraction: DoInteraction: done");
  }

} // namespace corsika::sibyll
