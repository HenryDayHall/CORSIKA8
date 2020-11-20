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

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <nuclib.hpp>

namespace corsika::sibyll {

  template <>
  NuclearInteraction<corsika::setup::SetupEnvironment>::NuclearInteraction(
      corsika::sibyll::Interaction& hadint, corsika::setup::SetupEnvironment const& env)
      : environment_(env)
      , hadronicInteraction_(hadint) {}

  template <>
  NuclearInteraction<corsika::setup::SetupEnvironment>::~NuclearInteraction() {
    std::cout << "Nuclib::NuclearInteraction n=" << count_ << " Nnuc=" << nucCount_
              << std::endl;
  }

  template <>
  void NuclearInteraction<corsika::setup::SetupEnvironment>::PrintCrossSectionTable(
      corsika::Code pCode) {
    const int k = targetComponentsIndex_.at(pCode);
    Code pNuclei[] = {Code::Helium, Code::Lithium7, Code::Oxygen,
                      Code::Neon,   Code::Argon,    Code::Iron};
    std::cout << "en/A ";
    for (auto& j : pNuclei) std::cout << std::setw(9) << j;
    std::cout << std::endl;

    // loop over energy bins
    for (unsigned int i = 0; i < GetNEnergyBins(); ++i) {
      std::cout << " " << i << "  ";
      for (auto& n : pNuclei) {
        auto const j = corsika::get_nucleus_A(n);
        std::cout << " " << std::setprecision(5) << std::setw(8)
                  << cnucsignuc_.sigma[j - 1][k][i];
      }
      std::cout << std::endl;
    }
  }

  template <>
  void
  NuclearInteraction<corsika::setup::SetupEnvironment>::InitializeNuclearCrossSections() {

    auto& universe = *(environment_.GetUniverse());

    auto const allElementsInUniverse = std::invoke([&]() {
      std::set<corsika::Code> allElementsInUniverse;
      auto collectElements = [&](auto& vtn) {
        if (vtn.HasModelProperties()) {
          auto const& comp =
              vtn.GetModelProperties().GetNuclearComposition().GetComponents();
          for (auto const c : comp) allElementsInUniverse.insert(c);
        }
      };
      universe.walk(collectElements);
      return allElementsInUniverse;
    });

    std::cout << "NuclearInteraction: initializing nuclear cross sections..."
              << std::endl;

    // loop over target components, at most 4!!
    int k = -1;
    for (auto& ptarg : allElementsInUniverse) {
      ++k;
      std::cout << "NuclearInteraction: init target component: " << ptarg << std::endl;
      const int ib = corsika::get_nucleus_A(ptarg);
      if (!hadronicInteraction_.IsValidTarget(ptarg)) {
        std::cout
            << "NuclearInteraction::InitializeNuclearCrossSections: target nucleus? id="
            << ptarg << std::endl;
        throw std::runtime_error(
            " target can not be handled by hadronic interaction model! ");
      }
      targetComponentsIndex_.insert(std::pair<Code, int>(ptarg, k));
      // loop over energies, fNEnBins log. energy bins
      for (unsigned int i = 0; i < GetNEnergyBins(); ++i) {
        // hard coded energy grid, has to be aligned to definition in signuc2!!, no
        // comment..
        const HEPEnergyType Ecm = pow(10., 1. + 1. * i) * 1_GeV;
        // get p-p cross sections
        auto const protonId = Code::Proton;
        auto const [siginel, sigela] =
            hadronicInteraction_.GetCrossSection(protonId, protonId, Ecm);
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
    std::cout << "NuclearInteraction: cross sections for "
              << targetComponentsIndex_.size() << " components initialized!" << std::endl;
    for (auto& ptarg : allElementsInUniverse) {
      std::cout << "cross section table: " << ptarg << std::endl;
      PrintCrossSectionTable(ptarg);
    }
  }

  template <>
  void NuclearInteraction<corsika::setup::SetupEnvironment>::Init() {

    // initialize hadronic interaction module
    // TODO: safe to run multiple initializations?
    if (!hadronicInteraction_.WasInitialized()) hadronicInteraction_.Init();

    // check compatibility of energy ranges, someone could try to use low-energy model..
    if (!hadronicInteraction_.IsValidCoMEnergy(GetMinEnergyPerNucleonCoM()) ||
        !hadronicInteraction_.IsValidCoMEnergy(GetMaxEnergyPerNucleonCoM()))
      throw std::runtime_error(
          "NuclearInteraction: hadronic interaction model incompatible!");

    // initialize nuclib
    // TODO: make sure this does not overlap with sibyll
    nuc_nuc_ini_();

    // initialize cross sections
    InitializeNuclearCrossSections();
  }

  template <>
  CrossSectionType
  NuclearInteraction<corsika::setup::SetupEnvironment>::ReadCrossSectionTable(
      const int ia, corsika::Code pTarget, HEPEnergyType elabnuc) {

    const int ib = targetComponentsIndex_.at(pTarget) + 1; // table index in fortran
    auto const ECoMNuc = sqrt(2. * constants::nucleonMass * elabnuc);
    if (ECoMNuc < GetMinEnergyPerNucleonCoM() || ECoMNuc > GetMaxEnergyPerNucleonCoM())
      throw std::runtime_error("NuclearInteraction: energy outside tabulated range!");
    const double e0 = elabnuc / 1_GeV;
    double sig;
    std::cout << "ReadCrossSectionTable: " << ia << " " << ib << " " << e0 << std::endl;
    signuc2_(ia, ib, e0, sig);
    std::cout << "ReadCrossSectionTable: sig=" << sig << std::endl;
    return sig * 1_mb;
  }

  // TODO: remove elastic cross section?
  template <>
  template <typename TParticle>
  std::tuple<CrossSectionType, CrossSectionType>
  NuclearInteraction<corsika::setup::SetupEnvironment>::GetCrossSection(
      const TParticle& vP, const corsika::Code TargetId) {

    if (vP.GetPID() != corsika::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: particle not a nucleus!");

    unsigned int const iBeamA = vP.GetNuclearA();
    HEPEnergyType LabEnergyPerNuc = vP.GetEnergy() / iBeamA;
    std::cout << "NuclearInteraction: GetCrossSection: called with: beamNuclA= " << iBeamA
              << " TargetId= " << TargetId
              << " LabEnergyPerNuc= " << LabEnergyPerNuc / 1_GeV << std::endl;

    // use nuclib to calc. nuclear cross sections
    // TODO: for now assumes air with hard coded composition
    // extend to arbitrary mixtures, requires smarter initialization
    // get nuclib projectile code: nucleon number
    if (iBeamA > GetMaxNucleusAProjectile() || iBeamA < 2) {
      std::cout << "NuclearInteraction: beam nucleus outside allowed range for NUCLIB!"
                << std::endl
                << "A=" << iBeamA << std::endl;
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: beam nucleus outside allowed range for "
          "NUCLIB!");
    }

    if (hadronicInteraction_.IsValidTarget(TargetId)) {
      auto const sigProd = ReadCrossSectionTable(iBeamA, TargetId, LabEnergyPerNuc);
      std::cout << "cross section (mb): " << sigProd / 1_mb << std::endl;
      return std::make_tuple(sigProd, 0_mb);
    } else {
      throw std::runtime_error("target outside range.");
    }
    return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mb,
                           std::numeric_limits<double>::infinity() * 1_mb);
  }

  template <>
  template <typename TParticle>
  GrammageType NuclearInteraction<corsika::setup::SetupEnvironment>::GetInteractionLength(
      const TParticle& vP) {

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::getInstance().GetRootCoordinateSystem();

    const corsika::Code corsikaBeamId = vP.GetPID();

    if (corsikaBeamId != corsika::Code::Nucleus) {
      // check if target-style nucleus (enum), these are not allowed as projectile
      if (corsika::is_nucleus(corsikaBeamId))
        throw std::runtime_error(
            "NuclearInteraction: GetInteractionLength: Wrong nucleus type. Nuclear "
            "projectiles should use NuclearStackExtension!");
      else {
        // no nuclear interaction
        return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
    }

    // read from cross section code table

    // FOR NOW: assume target is at rest
    corsika::MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.GetEnergy() + constants::nucleonMass;
    int const nuclA = vP.GetNuclearA();
    auto const ElabNuc = vP.GetEnergy() / nuclA;

    corsika::MomentumVector pTotLab(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    pTotLab += vP.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy
    auto const ECoMNN = sqrt(2. * ElabNuc * constants::nucleonMass);
    std::cout << "NuclearInteraction: LambdaInt: \n"
              << " input energy: " << Elab / 1_GeV << std::endl
              << " input energy CoM: " << ECoM / 1_GeV << std::endl
              << " beam pid:" << corsikaBeamId << std::endl
              << " beam A: " << nuclA << std::endl
              << " input energy per nucleon: " << ElabNuc / 1_GeV << std::endl
              << " input energy CoM per nucleon: " << ECoMNN / 1_GeV << std::endl;
    //      throw std::runtime_error("stop here");

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
      auto const* const currentNode = vP.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();
      // determine average interaction length
      // weighted sum
      int i = -1;
      CrossSectionType weightedProdCrossSection = 0_mb;
      // get weights of components from environment/medium
      const auto& w = mediumComposition.GetFractions();
      // loop over components in medium
      for (auto const targetId : mediumComposition.GetComponents()) {
        i++;
        std::cout << "NuclearInteraction: get interaction length for target: " << targetId
                  << std::endl;
        auto const [productionCrossSection, elaCrossSection] =
            GetCrossSection(vP, targetId);
        [[maybe_unused]] auto& dummy_elaCrossSection = elaCrossSection;

        std::cout << "NuclearInteraction: "
                  << "IntLength: nuclib return (mb): " << productionCrossSection / 1_mb
                  << std::endl;
        weightedProdCrossSection += w[i] * productionCrossSection;
      }
      std::cout << "NuclearInteraction: "
                << "IntLength: weighted CrossSection (mb): "
                << weightedProdCrossSection / 1_mb << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      std::cout << "NuclearInteraction: "
                << "interaction length (g/cm2): "
                << int_length * (1_cm * 1_cm / (0.001_kg)) << std::endl;

      return int_length;
    } else {
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }
  }

  template <>
  template <typename TProjectile>
  corsika::EProcessReturn
  NuclearInteraction<corsika::setup::SetupEnvironment>::DoInteraction(TProjectile& vP) {

    using namespace si;

    // this routine superimposes different nucleon-nucleon interactions
    // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC

    const auto ProjId = vP.GetPID();
    // TODO: calculate projectile mass in nuclearStackExtension
    //      const auto ProjMass = vP.GetMass();
    std::cout << "NuclearInteraction: DoInteraction: called with:" << ProjId << std::endl;

    // check if target-style nucleus (enum)
    if (ProjId != corsika::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: DoInteraction: Wrong nucleus type. Nuclear projectiles "
          "should use NuclearStackExtension!");

    auto const ProjMass = corsika::nucleus_mass(vP.GetNuclearA(), vP.GetNuclearZ());
    std::cout << "NuclearInteraction: projectile mass: " << ProjMass / 1_GeV << std::endl;

    count_++;

    const CoordinateSystem& rootCS =
        RootCoordinateSystem::getInstance().GetRootCoordinateSystem();

    // position and time of interaction, not used in NUCLIB
    Point pOrig = vP.GetPosition();
    TimeType tOrig = vP.GetTime();

    std::cout << "Interaction: position of interaction: " << pOrig.GetCoordinates()
              << std::endl;
    std::cout << "Interaction: time: " << tOrig << std::endl;

    // projectile nucleon number
    const unsigned int kAProj = vP.GetNuclearA();
    if (kAProj > GetMaxNucleusAProjectile())
      throw std::runtime_error("Projectile nucleus too large for NUCLIB!");

    // kinematics
    // define projectile nucleus
    HEPEnergyType const eProjectileLab = vP.GetEnergy();
    auto const pProjectileLab = vP.GetMomentum();
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    std::cout << "NuclearInteraction: eProj lab: " << eProjectileLab / 1_GeV << std::endl
              << "NuclearInteraction: pProj lab: "
              << pProjectileLab.GetComponents() / 1_GeV << std::endl;

    // define projectile nucleon
    HEPEnergyType const eProjectileNucLab = vP.GetEnergy() / kAProj;
    auto const pProjectileNucLab = vP.GetMomentum() / kAProj;
    const FourVector PprojNucLab(eProjectileNucLab, pProjectileNucLab);

    std::cout << "NuclearInteraction: eProjNucleon lab: " << eProjectileNucLab / 1_GeV
              << std::endl
              << "NuclearInteraction: pProjNucleon lab: "
              << pProjectileNucLab.GetComponents() / 1_GeV << std::endl;

    // define target
    // always a nucleon
    // target is always at rest
    const auto eTargetNucLab = 0_GeV + constants::nucleonMass;
    const auto pTargetNucLab = corsika::MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
    const FourVector PtargNucLab(eTargetNucLab, pTargetNucLab);

    std::cout << "NuclearInteraction: etarget lab: " << eTargetNucLab / 1_GeV << std::endl
              << "NuclearInteraction: ptarget lab: "
              << pTargetNucLab.GetComponents() / 1_GeV << std::endl;

    // center-of-mass energy in nucleon-nucleon frame
    auto const PtotNN4 = PtargNucLab + PprojNucLab;
    HEPEnergyType EcmNN = PtotNN4.GetNorm();
    std::cout << "NuclearInteraction: nuc-nuc cm energy: " << EcmNN / 1_GeV << std::endl;

    if (!hadronicInteraction_.IsValidCoMEnergy(EcmNN)) {
      std::cout << "NuclearInteraction: nuc-nuc. CoM energy too low for hadronic "
                   "interaction model!"
                << std::endl;
      throw std::runtime_error("NuclearInteraction: DoInteraction: energy too low!");
    }

    // define boost to NUCLEON-NUCLEON frame
    COMBoost const boost(PprojNucLab, constants::nucleonMass);
    // boost projecticle
    auto const PprojNucCoM = boost.toCoM(PprojNucLab);

    // boost target
    auto const PtargNucCoM = boost.toCoM(PtargNucLab);

    std::cout << "Interaction: ebeam CoM: " << PprojNucCoM.GetTimeLikeComponent() / 1_GeV
              << std::endl
              << "Interaction: pbeam CoM: "
              << PprojNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV
              << std::endl;
    std::cout << "Interaction: etarget CoM: "
              << PtargNucCoM.GetTimeLikeComponent() / 1_GeV << std::endl
              << "Interaction: ptarget CoM: "
              << PtargNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV
              << std::endl;

    // sample target nucleon number
    //
    // proton stand-in for nucleon
    const auto beamId = corsika::Code::Proton;
    auto const* const currentNode = vP.GetNode();
    const auto& mediumComposition =
        currentNode->GetModelProperties().GetNuclearComposition();
    std::cout << "get nucleon-nucleus cross sections for target materials.." << std::endl;
    // get cross sections for target materials
    // using nucleon-target-nucleus cross section!!!
    /*
      Here we read the cross section from the interaction model again,
      should be passed from GetInteractionLength if possible
    */
    auto const& compVec = mediumComposition.GetComponents();
    std::vector<CrossSectionType> cross_section_of_components(compVec.size());

    for (size_t i = 0; i < compVec.size(); ++i) {
      auto const targetId = compVec[i];
      std::cout << "target component: " << targetId << std::endl;
      std::cout << "beam id: " << beamId << std::endl;
      const auto [sigProd, sigEla] =
          hadronicInteraction_.GetCrossSection(beamId, targetId, EcmNN);
      cross_section_of_components[i] = sigProd;
      [[maybe_unused]] auto sigElaCopy = sigEla; // ONLY TO AVOID COMPILER WARNINGS
    }

    const auto targetCode =
        mediumComposition.SampleTarget(cross_section_of_components, RNG_);
    std::cout << "Interaction: target selected: " << targetCode << std::endl;
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int kATarget = -1;
    if (corsika::is_nucleus(targetCode))
      kATarget = corsika::get_nucleus_A(targetCode);
    else if (targetCode == corsika::Code::Proton)
      kATarget = 1;
    std::cout << "NuclearInteraction: nuclib target code: " << kATarget << std::endl;
    if (!hadronicInteraction_.IsValidTarget(targetCode))
      throw std::runtime_error("target outside range. ");
    // end of target sampling

    // superposition
    std::cout << "NuclearInteraction: sampling nuc. multiple interaction structure.. "
              << std::endl;
    // get nucleon-nucleon cross section
    // (needed to determine number of nucleon-nucleon scatterings)
    const auto protonId = corsika::Code::Proton;
    const auto [prodCrossSection, elaCrossSection] =
        hadronicInteraction_.GetCrossSection(protonId, protonId, EcmNN);
    const double sigProd = prodCrossSection / 1_mb;
    const double sigEla = elaCrossSection / 1_mb;
    // sample number of interactions (only input variables, output in common cnucms)
    // nuclear multiple scattering according to glauber (r.i.p.)
    int_nuc_(kATarget, kAProj, sigProd, sigEla);

    std::cout << "number of nucleons in target           : " << kATarget << std::endl
              << "number of wounded nucleons in target   : " << cnucms_.na << std::endl
              << "number of nucleons in projectile       : " << kAProj << std::endl
              << "number of wounded nucleons in project. : " << cnucms_.nb << std::endl
              << "number of inel. nuc.-nuc. interactions : " << cnucms_.ni << std::endl
              << "number of elastic nucleons in target   : " << cnucms_.nael << std::endl
              << "number of elastic nucleons in project. : " << cnucms_.nbel << std::endl
              << "impact parameter: " << cnucms_.b << std::endl;

    // calculate fragmentation
    std::cout << "calculating nuclear fragments.." << std::endl;
    // number of interactions
    // include elastic
    const int nElasticNucleons = cnucms_.nbel;
    const int nInelNucleons = cnucms_.nb;
    const int nIntProj = nInelNucleons + nElasticNucleons;
    const double impactPar = cnucms_.b; // only needed to avoid passing common var.
    int nFragments;
    // number of fragments is limited to 60
    int AFragments[60];
    // call fragmentation routine
    // input: target A, projectile A, number of int. nucleons in projectile, impact
    // parameter (fm) output: nFragments, AFragments in addition the momenta ar stored
    // in pf in common fragments, neglected
    fragm_(kATarget, kAProj, nIntProj, impactPar, nFragments, AFragments);

    // this should not occur but well :)
    if (nFragments > (int)GetMaxNFragments())
      throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");

    std::cout << "number of fragments: " << nFragments << std::endl;
    for (int j = 0; j < nFragments; ++j)
      std::cout << "fragment: " << j << " A=" << AFragments[j]
                << " px=" << fragments_.ppp[j][0] << " py=" << fragments_.ppp[j][1]
                << " pz=" << fragments_.ppp[j][2] << std::endl;

    std::cout << "adding nuclear fragments to particle stack.." << std::endl;
    // put nuclear fragments on corsika stack
    for (int j = 0; j < nFragments; ++j) {
      corsika::Code specCode;
      const auto nuclA = AFragments[j];
      // get Z from stability line
      const auto nuclZ = int(nuclA / 2.15 + 0.7);

      // TODO: do we need to catch single nucleons??
      if (nuclA == 1)
        // TODO: sample neutron or proton
        specCode = corsika::Code::Proton;
      else
        specCode = corsika::Code::Nucleus;

      const HEPMassType mass = corsika::nucleus_mass(nuclA, nuclZ);

      std::cout << "NuclearInteraction: adding fragment: " << specCode << std::endl;
      std::cout << "NuclearInteraction: A,Z: " << nuclA << "," << nuclZ << std::endl;
      std::cout << "NuclearInteraction: mass: " << mass / 1_GeV << std::endl;

      // CORSIKA 7 way
      // spectators inherit momentum from original projectile
      const double mass_ratio = mass / ProjMass;

      std::cout << "NuclearInteraction: mass ratio " << mass_ratio << std::endl;

      auto const Plab = PprojLab * mass_ratio;

      std::cout << "NuclearInteraction: fragment momentum: "
                << Plab.GetSpaceLikeComponents().GetComponents() / 1_GeV << std::endl;

      if (nuclA == 1)
        // add nucleon
        vP.AddSecondary(std::tuple<corsika::Code, si::HEPEnergyType,
                                   corsika::MomentumVector, corsika::Point, si::TimeType>{
            specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
            tOrig});
      else
        // add nucleus
        vP.AddSecondary(
            std::tuple<corsika::Code, si::HEPEnergyType, corsika::MomentumVector,
                       corsika::Point, si::TimeType, unsigned short, unsigned short>{
                specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
                pOrig, tOrig, nuclA, nuclZ});
    }

    // add elastic nucleons to corsika stack
    // TODO: the elastic interaction could be external like the inelastic interaction,
    // e.g. use existing ElasticModel
    std::cout << "adding elastically scattered nucleons to particle stack.." << std::endl;
    for (int j = 0; j < nElasticNucleons; ++j) {
      // TODO: sample proton or neutron
      auto const elaNucCode = corsika::Code::Proton;

      // CORSIKA 7 way
      // elastic nucleons inherit momentum from original projectile
      // neglecting momentum transfer in interaction
      const double mass_ratio = corsika::get_mass(elaNucCode) / ProjMass;
      auto const Plab = PprojLab * mass_ratio;

      vP.AddSecondary(std::tuple<corsika::Code, si::HEPEnergyType,
                                 corsika::MomentumVector, corsika::Point, si::TimeType>{
          elaNucCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
          tOrig});
    }

    // add inelastic interactions
    std::cout << "calculate inelastic nucleon-nucleon interactions.." << std::endl;
    for (int j = 0; j < nInelNucleons; ++j) {
      // TODO: sample neutron or proton
      auto pCode = corsika::Code::Proton;
      // temporarily add to stack, will be removed after interaction in DoInteraction
      std::cout << "inelastic interaction no. " << j << std::endl;
      auto inelasticNucleon = vP.AddSecondary(
          std::tuple<corsika::Code, si::HEPEnergyType, corsika::MomentumVector,
                     corsika::Point, si::TimeType>{
              pCode, PprojNucLab.GetTimeLikeComponent(),
              PprojNucLab.GetSpaceLikeComponents(), pOrig, tOrig});
      // create inelastic interaction
      std::cout << "calling HadronicInteraction..." << std::endl;
      hadronicInteraction_.DoInteraction(inelasticNucleon);
    }

    std::cout << "NuclearInteraction: DoInteraction: done" << std::endl;

    return corsika::EProcessReturn::eOk;
  }

} // namespace corsika::sibyll
