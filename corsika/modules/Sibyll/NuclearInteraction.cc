/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/process/sibyll/nuclib.h>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <set>
#include <sstream>

#include "../../corsika/setup/SetupStack.hpp"
#include "../../corsika/setup/SetupTrajectory.hpp"

using std::cout;
using std::endl;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika;
using Particle = Stack::ParticleType;        // StackIterator; // ParticleType;
using Projectile = StackView::StackIterator; // StackView::ParticleType;
using Track = Trajectory;

namespace corsika::sibyll {

  template <>
  NuclearInteraction<setup::Environment>::~NuclearInteraction() {
    C8LOG_DEBUG(
        fmt::format("Nuclib::NuclearInteraction n={} Nnuc={}", count_, nucCount_));
  }

  template <>
  void NuclearInteraction<SetupEnvironment>::PrintCrossSectionTable(
      corsika::Code pCode) {
    using namespace corsika;
    const int k = targetComponentsIndex_.at(pCode);
    Code pNuclei[] = {Code::Helium, Code::Lithium7, Code::Oxygen,
                      Code::Neon,   Code::Argon,    Code::Iron};
    std::ostringstream table;
    table << "Nuclear CrossSectionTable pCode=" << pCode << " :\n en/A ";
    for (auto& j : pNuclei) table << std::setw(9) << j;
    table << "\n";

    // loop over energy bins
    for (unsigned int i = 0; i < GetNEnergyBins(); ++i) {
      table << " " << i << "  ";
      for (auto& n : pNuclei) {
        auto const j = GetNucleusA(n);
        table << " " << std::setprecision(5) << std::setw(8)
              << cnucsignuc_.sigma[j - 1][k][i];
      }
      table << "\n";
    }
    C8LOG_DEBUG(table.str());
  }

  template <>
  void NuclearInteraction<SetupEnvironment>::InitializeNuclearCrossSections() {
    using namespace corsika;
    using namespace units::si;

    auto& universe = *(environment_.GetUniverse());

    auto const allElementsInUniverse = std::invoke([&]() {
      std::set<particles::Code> allElementsInUniverse;
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

    C8LOG_DEBUG("NuclearInteraction: initializing nuclear cross sections...");

    // loop over target components, at most 4!!
    int k = -1;
    for (auto& ptarg : allElementsInUniverse) {
      ++k;
      C8LOG_DEBUG(fmt::format("NuclearInteraction: init target component: {}", ptarg));
      const int ib = GetNucleusA(ptarg);
      if (!hadronicInteraction_.IsValidTarget(ptarg)) {
        C8LOG_DEBUG(fmt::format(
            "NuclearInteraction::InitializeNuclearCrossSections: target nucleus? id={}",
            ptarg));
        throw std::runtime_error(
            " target can not be handled by hadronic interaction model! ");
      }
      targetComponentsIndex_.insert(std::pair<Code, int>(ptarg, k));
      // loop over energies, fNEnBins log. energy bins
      for (unsigned int i = 0; i < GetNEnergyBins(); ++i) {
        // hard coded energy grid, has to be aligned to definition in signuc2!!, no
        // comment..
        const units::si::HEPEnergyType Ecm = pow(10., 1. + 1. * i) * 1_GeV;
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
    C8LOG_DEBUG(
        fmt::format("NuclearInteraction: cross sections for {} "
                    " components initialized!",
                    targetComponentsIndex_.size()));
    for (auto& ptarg : allElementsInUniverse) { PrintCrossSectionTable(ptarg); }
  }

  template <>
  units::si::CrossSectionType
  NuclearInteraction<setup::Environment>::ReadCrossSectionTable(
      const int ia, particles::Code pTarget, units::si::HEPEnergyType elabnuc) {
    using namespace corsika;
    using namespace units::si;
    const int ib = targetComponentsIndex_.at(pTarget) + 1; // table index in fortran
    auto const ECoMNuc = sqrt(2. * corsika::units::constants::nucleonMass * elabnuc);
    if (ECoMNuc < GetMinEnergyPerNucleonCoM() || ECoMNuc > GetMaxEnergyPerNucleonCoM())
      throw std::runtime_error("NuclearInteraction: energy outside tabulated range!");
    const double e0 = elabnuc / 1_GeV;
    double sig;
    C8LOG_DEBUG(fmt::format("ReadCrossSectionTable: {} {} {}", ia, ib, e0));
    signuc2_(ia, ib, e0, sig);
    C8LOG_DEBUG(fmt::format("ReadCrossSectionTable: sig={}", sig));
    return sig * 1_mb;
  }

  // TODO: remove elastic cross section?
  template <>
  template <>
  tuple<units::si::CrossSectionType, units::si::CrossSectionType>
  NuclearInteraction<setup::Environment>::GetCrossSection(
      Particle const& vP, const particles::Code TargetId) {
    using namespace units::si;
    if (vP.GetPID() != particles::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: particle not a nucleus!");

    const unsigned int iBeamA = vP.GetNuclearA();
    HEPEnergyType LabEnergyPerNuc = vP.GetEnergy() / iBeamA;
    C8LOG_DEBUG(
        fmt::format("NuclearInteraction: GetCrossSection: called with: beamNuclA={} "
                    " TargetId={} LabEnergyPerNuc={}GeV ",
                    iBeamA, TargetId, LabEnergyPerNuc / 1_GeV));

    // use nuclib to calc. nuclear cross sections
    // TODO: for now assumes air with hard coded composition
    // extend to arbitrary mixtures, requires smarter initialization
    // get nuclib projectile code: nucleon number
    if (iBeamA > GetMaxNucleusAProjectile() || iBeamA < 2) {
      C8LOG_DEBUG(
          "NuclearInteraction: beam nucleus outside allowed range for NUCLIB!"
          "A=" +
          std::to_string(iBeamA));
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: beam nucleus outside allowed range for "
          "NUCLIB!");
    }

    if (hadronicInteraction_.IsValidTarget(TargetId)) {
      auto const sigProd = ReadCrossSectionTable(iBeamA, TargetId, LabEnergyPerNuc);
      C8LOG_DEBUG("cross section (mb): " + std::to_string(sigProd / 1_mb));
      return std::make_tuple(sigProd, 0_mb);
    } else {
      throw std::runtime_error("target outside range.");
    }
    return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mb,
                           std::numeric_limits<double>::infinity() * 1_mb);
  }

  template <>
  template <>
  units::si::GrammageType NuclearInteraction<setup::Environment>::GetInteractionLength(
      Particle const& vP) {

    using namespace units;
    using namespace units::si;
    using namespace geometry;

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    const particles::Code corsikaBeamId = vP.GetPID();

    if (corsikaBeamId != particles::Code::Nucleus) {
      // check if target-style nucleus (enum), these are not allowed as projectile
      if (particles::IsNucleus(corsikaBeamId))
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
    unsigned int const nuclA = vP.GetNuclearA();
    auto const ElabNuc = vP.GetEnergy() / nuclA;

    corsika::MomentumVector pTotLab(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    pTotLab += vP.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    [[maybe_unused]] const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy
    auto const ECoMNN = sqrt(2. * ElabNuc * constants::nucleonMass);
    C8LOG_DEBUG(
        fmt::format("NuclearInteraction: LambdaInt: \n"
                    " input energy: {}GeV\n"
                    " input energy CoM: {}GeV\n"
                    " beam pid: {}\n"
                    " beam A: {}\n"
                    " input energy per nucleon: {}GeV\n"
                    " input energy CoM per nucleon: {}GeV ",
                    Elab / 1_GeV, ECoM / 1_GeV, particles::GetName(corsikaBeamId), nuclA,
                    ElabNuc / 1_GeV, ECoMNN / 1_GeV));
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
      si::CrossSectionType weightedProdCrossSection = 0_mb;
      // get weights of components from environment/medium
      const auto& w = mediumComposition.GetFractions();
      // loop over components in medium
      for (auto const targetId : mediumComposition.GetComponents()) {
        i++;
        C8LOG_DEBUG("NuclearInteraction: get interaction length for target: " +
                    particles::GetName(targetId));
        auto const [productionCrossSection, elaCrossSection] =
            GetCrossSection(vP, targetId);
        [[maybe_unused]] auto& dummy_elaCrossSection = elaCrossSection;

        C8LOG_DEBUG(
            "NuclearInteraction: "
            "IntLength: nuclib return (mb): " +
            std::to_string(productionCrossSection / 1_mb));
        weightedProdCrossSection += w[i] * productionCrossSection;
      }
      C8LOG_DEBUG(
          "NuclearInteraction: "
          "IntLength: weighted CrossSection (mb): " +
          std::to_string(weightedProdCrossSection / 1_mb));

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      C8LOG_DEBUG(
          "NuclearInteraction: "
          "interaction length (g/cm2): " +
          std::to_string(int_length * (1_cm * 1_cm / (0.001_kg))));

      return int_length;
    } else {
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }
  }

  template <>
  template <>
  process::EProcessReturn NuclearInteraction<setup::Environment>::DoInteraction(
      View& view) {

    // this routine superimposes different nucleon-nucleon interactions
    // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC

    using namespace units;
    using namespace utl;
    using namespace units::si;
    using namespace geometry;

    auto projectile = view.GetProjectile();

    const auto ProjId = projectile.GetPID();
    // TODO: calculate projectile mass in nuclearStackExtension
    //      const auto ProjMass = projectile.GetMass();
    C8LOG_DEBUG("NuclearInteraction: DoInteraction: called with:" +
                particles::GetName(ProjId));

    // check if target-style nucleus (enum)
    if (ProjId != particles::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: DoInteraction: Wrong nucleus type. Nuclear projectiles "
          "should use NuclearStackExtension!");

    auto const ProjMass = projectile.GetNuclearZ() * particles::Proton::GetMass() +
                          (projectile.GetNuclearA() - projectile.GetNuclearZ()) *
                              particles::Neutron::GetMass();
    C8LOG_DEBUG("NuclearInteraction: projectile mass: " +
                std::to_string(ProjMass / 1_GeV));

    count_++;

    const CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    // position and time of interaction, not used in NUCLIB
    Point pOrig = projectile.GetPosition();
    TimeType tOrig = projectile.GetTime();

    C8LOG_DEBUG(
        fmt::format("Interaction: position of interaction: {}", pOrig.GetCoordinates()));
    C8LOG_DEBUG("Interaction: time: " + std::to_string(tOrig / 1_s));

    // projectile nucleon number
    const unsigned int kAProj = projectile.GetNuclearA();
    if (kAProj > GetMaxNucleusAProjectile())
      throw std::runtime_error("Projectile nucleus too large for NUCLIB!");

    // kinematics
    // define projectile nucleus
    HEPEnergyType const eProjectileLab = projectile.GetEnergy();
    auto const pProjectileLab = projectile.GetMomentum();
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    C8LOG_DEBUG(
        fmt::format("NuclearInteraction: eProj lab: {} "
                    "pProj lab: {} ",
                    eProjectileLab / 1_GeV, pProjectileLab.GetComponents() / 1_GeV));
    ;

    // define projectile nucleon
    HEPEnergyType const eProjectileNucLab = projectile.GetEnergy() / kAProj;
    auto const pProjectileNucLab = projectile.GetMomentum() / kAProj;
    const FourVector PprojNucLab(eProjectileNucLab, pProjectileNucLab);

    C8LOG_DEBUG(fmt::format(
        "NuclearInteraction: eProjNucleon lab (GeV): {} "
        "pProjNucleon lab (GeV): {}",
        eProjectileNucLab / 1_GeV, pProjectileNucLab.GetComponents() / 1_GeV));

    // define target
    // always a nucleon
    // target is always at rest
    const auto eTargetNucLab = 0_GeV + constants::nucleonMass;
    const auto pTargetNucLab =
        corsika::MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
    const FourVector PtargNucLab(eTargetNucLab, pTargetNucLab);

    C8LOG_DEBUG(
        fmt::format("NuclearInteraction: etarget lab(GeV): {}"
                    "NuclearInteraction: ptarget lab(GeV): {} ",
                    eTargetNucLab / 1_GeV, pTargetNucLab.GetComponents() / 1_GeV));

    // center-of-mass energy in nucleon-nucleon frame
    auto const PtotNN4 = PtargNucLab + PprojNucLab;
    HEPEnergyType EcmNN = PtotNN4.GetNorm();
    C8LOG_DEBUG("NuclearInteraction: nuc-nuc cm energy: " +
                std::to_string(EcmNN / 1_GeV));

    if (!hadronicInteraction_.IsValidCoMEnergy(EcmNN)) {
      C8LOG_DEBUG(
          "NuclearInteraction: nuc-nuc. CoM energy too low for hadronic "
          "interaction model!");
      throw std::runtime_error("NuclearInteraction: DoInteraction: energy too low!");
    }

    // define boost to NUCLEON-NUCLEON frame
    COMBoost const boost(PprojNucLab, constants::nucleonMass);
    // boost projecticle
    [[maybe_unused]] auto const PprojNucCoM = boost.toCoM(PprojNucLab);

    // boost target
    [[maybe_unused]] auto const PtargNucCoM = boost.toCoM(PtargNucLab);

    C8LOG_DEBUG(
        fmt::format("Interaction: ebeam CoM: {} "
                    ", pbeam CoM: {}",
                    PprojNucCoM.GetTimeLikeComponent() / 1_GeV,
                    PprojNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV));
    C8LOG_DEBUG(
        fmt::format("Interaction: etarget CoM: {}"
                    ", ptarget CoM: {}",
                    PtargNucCoM.GetTimeLikeComponent() / 1_GeV,
                    PtargNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV));

    // sample target nucleon number
    //
    // proton stand-in for nucleon
    const auto beamId = particles::Proton::GetCode();
    auto const* const currentNode = projectile.GetNode();
    const auto& mediumComposition =
        currentNode->GetModelProperties().GetNuclearComposition();
    C8LOG_DEBUG("get nucleon-nucleus cross sections for target materials..");
    // get cross sections for target materials
    // using nucleon-target-nucleus cross section!!!
    /*
      Here we read the cross section from the interaction model again,
      should be passed from GetInteractionLength if possible
    */
    auto const& compVec = mediumComposition.GetComponents();
    vector<si::CrossSectionType> cross_section_of_components(compVec.size());

    for (size_t i = 0; i < compVec.size(); ++i) {
      auto const targetId = compVec[i];
      C8LOG_DEBUG("target component: " + particles::GetName(targetId));
      C8LOG_DEBUG("beam id: " + particles::GetName(beamId));
      const auto [sigProd, sigEla] =
          hadronicInteraction_.GetCrossSection(beamId, targetId, EcmNN);
      cross_section_of_components[i] = sigProd;
      [[maybe_unused]] auto sigElaCopy = sigEla; // ONLY TO AVOID COMPILER WARNINGS
    }

    const auto targetCode =
        mediumComposition.SampleTarget(cross_section_of_components, RNG_);
    C8LOG_DEBUG("Interaction: target selected: " + particles::GetName(targetCode));
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int kATarget = -1;
    if (IsNucleus(targetCode)) kATarget = GetNucleusA(targetCode);
    if (targetCode == particles::Proton::GetCode()) kATarget = 1;
    C8LOG_DEBUG("NuclearInteraction: nuclib target code: " + std::to_string(kATarget));
    if (!hadronicInteraction_.IsValidTarget(targetCode))
      throw std::runtime_error("target outside range. ");
    // end of target sampling

    // superposition
    C8LOG_DEBUG("NuclearInteraction: sampling nuc. multiple interaction structure.. ");
    // get nucleon-nucleon cross section
    // (needed to determine number of nucleon-nucleon scatterings)
    const auto protonId = particles::Proton::GetCode();
    const auto [prodCrossSection, elaCrossSection] =
        hadronicInteraction_.GetCrossSection(protonId, protonId, EcmNN);
    const double sigProd = prodCrossSection / 1_mb;
    const double sigEla = elaCrossSection / 1_mb;
    // sample number of interactions (only input variables, output in common cnucms)
    // nuclear multiple scattering according to glauber (r.i.p.)
    int_nuc_(kATarget, kAProj, sigProd, sigEla);

    C8LOG_DEBUG(
        fmt::format("number of nucleons in target           : {}\n"
                    "number of wounded nucleons in target   : {}\n"
                    "number of nucleons in projectile       : {}\n"
                    "number of wounded nucleons in project. : {}\n"
                    "number of inel. nuc.-nuc. interactions : {}\n"
                    "number of elastic nucleons in target   : {}\n"
                    "number of elastic nucleons in project. : {}\n"
                    "impact parameter: {}",
                    kATarget, cnucms_.na, kAProj, cnucms_.nb, cnucms_.ni, cnucms_.nael,
                    cnucms_.nbel, cnucms_.b));

    // calculate fragmentation
    C8LOG_DEBUG("calculating nuclear fragments..");
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

    // this should not occur but well :)
    if (nFragments > (int)GetMaxNFragments())
      throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");

    C8LOG_DEBUG("number of fragments: " + std::to_string(nFragments));
    for (int j = 0; j < nFragments; ++j)
      C8LOG_DEBUG(fmt::format("fragment {}: A={} px={} py={} pz={}", j, AFragments[j],
                              fragments_.ppp[j][0], fragments_.ppp[j][1],
                              fragments_.ppp[j][2]));

    C8LOG_DEBUG("adding nuclear fragments to particle stack..");
    // put nuclear fragments on corsika stack
    for (int j = 0; j < nFragments; ++j) {
      particles::Code specCode;
      const int nuclA = AFragments[j];
      // get Z from stability line
      const int nuclZ = int(nuclA / 2.15 + 0.7);

      // TODO: do we need to catch single nucleons??
      if (nuclA == 1)
        // TODO: sample neutron or proton
        specCode = particles::Code::Proton;
      else
        specCode = particles::Code::Nucleus;

      // TODO: mass of nuclei?
      const HEPMassType mass =
          particles::Proton::GetMass() * nuclZ +
          (nuclA - nuclZ) * particles::Neutron::GetMass(); // this neglects binding energy

      C8LOG_DEBUG("NuclearInteraction: adding fragment: " + particles::GetName(specCode));
      C8LOG_DEBUG("NuclearInteraction: A,Z: " + std::to_string(nuclA) + ", " +
                  std::to_string(nuclZ));
      C8LOG_DEBUG("NuclearInteraction: mass: " + std::to_string(mass / 1_GeV));

      // CORSIKA 7 way
      // spectators inherit momentum from original projectile
      const double mass_ratio = mass / ProjMass;

      C8LOG_DEBUG("NuclearInteraction: mass ratio " + std::to_string(mass_ratio));

      auto const Plab = PprojLab * mass_ratio;

      C8LOG_DEBUG(fmt::format("NuclearInteraction: fragment momentum: {}",
                              Plab.GetSpaceLikeComponents().GetComponents() / 1_GeV));

      if (nuclA == 1)
        // add nucleon
        projectile.AddSecondary(make_tuple(specCode, Plab.GetTimeLikeComponent(),
                                           Plab.GetSpaceLikeComponents(), pOrig, tOrig));
      else
        // add nucleus
        vP.AddSecondary(tuple<particles::Code, units::si::HEPEnergyType,
                              corsika::MomentumVector, geometry::Point,
                              units::si::TimeType, unsigned short, unsigned short>{
            specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
            tOrig, nuclA, nuclZ});
    }

    // add elastic nucleons to corsika stack
    // TODO: the elastic interaction could be external like the inelastic interaction,
    // e.g. use existing ElasticModel
    C8LOG_DEBUG("adding elastically scattered nucleons to particle stack..");
    for (int j = 0; j < nElasticNucleons; ++j) {
      // TODO: sample proton or neutron
      auto const elaNucCode = particles::Code::Proton;

      // CORSIKA 7 way
      // elastic nucleons inherit momentum from original projectile
      // neglecting momentum transfer in interaction
      const double mass_ratio = particles::GetMass(elaNucCode) / ProjMass;
      auto const Plab = PprojLab * mass_ratio;

      vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                geometry::Point, units::si::TimeType>{
              elaNucCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
              pOrig, tOrig});
    }

    // add inelastic interactions
    C8LOG_DEBUG("calculate inelastic nucleon-nucleon interactions..");
    for (int j = 0; j < nInelNucleons; ++j) {
      // TODO: sample neutron or proton
      auto pCode = particles::Proton::GetCode();
      // temporarily add to stack, will be removed after interaction in DoInteraction
      cout << "inelastic interaction no. " << j << endl;
      auto inelasticNucleon = vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                geometry::Point, units::si::TimeType>{
              pCode, PprojNucLab.GetTimeLikeComponent(),
              PprojNucLab.GetSpaceLikeComponents(), pOrig, tOrig});
      // create inelastic interaction
      cout << "calling HadronicInteraction..." << endl;
      hadronicInteraction_.DoInteraction(inelasticNucleon);
    }

    C8LOG_DEBUG("NuclearInteraction: DoInteraction: done");

    return process::EProcessReturn::eOk;
  }

} // namespace corsika::sibyll
