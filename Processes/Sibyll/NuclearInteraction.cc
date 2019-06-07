/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/sibyll/Interaction.h>
#include <corsika/process/sibyll/NuclearInteraction.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/process/sibyll/nuclib.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <set>

using std::cout;
using std::endl;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika::setup;
using Particle = Stack::ParticleType;        // StackIterator; // ParticleType;
using Projectile = StackView::StackIterator; // StackView::ParticleType;
using Track = Trajectory;

namespace corsika::process::sibyll {

  template <>
  NuclearInteraction<SetupEnvironment>::NuclearInteraction(
      process::sibyll::Interaction& hadint, SetupEnvironment const& env)
      : fEnvironment(env)
      , fHadronicInteraction(hadint) {}

  template <>
  NuclearInteraction<SetupEnvironment>::~NuclearInteraction() {
    cout << "Nuclib::NuclearInteraction n=" << fCount << " Nnuc=" << fNucCount << endl;
  }

  template <>
  void NuclearInteraction<SetupEnvironment>::PrintCrossSectionTable(
      corsika::particles::Code pCode) {
    using namespace corsika::particles;
    const int k = fTargetComponentsIndex.at(pCode);
    Code pNuclei[] = {Code::Helium, Code::Lithium7, Code::Oxygen,
                      Code::Neon,   Code::Argon,    Code::Iron};
    cout << "en/A ";
    for (auto& j : pNuclei) cout << std::setw(9) << j;
    cout << endl;

    // loop over energy bins
    for (int i = 0; i < GetNEnergyBins(); ++i) {
      cout << " " << i << "  ";
      for (auto& n : pNuclei) {
        auto const j = GetNucleusA(n);
        cout << " " << std::setprecision(5) << std::setw(8)
             << cnucsignuc_.sigma[j - 1][k][i];
      }
      cout << endl;
    }
  }

  template <>
  void NuclearInteraction<SetupEnvironment>::InitializeNuclearCrossSections() {
    using namespace corsika::particles;
    using namespace units::si;

    auto& universe = *(fEnvironment.GetUniverse());

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

    cout << "NuclearInteraction: initializing nuclear cross sections..." << endl;

    // loop over target components, at most 4!!
    int k = -1;
    for (auto& ptarg : allElementsInUniverse) {
      ++k;
      cout << "NuclearInteraction: init target component: " << ptarg << endl;
      const int ib = GetNucleusA(ptarg);
      if (!fHadronicInteraction.IsValidTarget(ptarg)) {
        cout << "NuclearInteraction::InitializeNuclearCrossSections: target nucleus? id="
             << ptarg << endl;
        throw std::runtime_error(
            " target can not be handled by hadronic interaction model! ");
      }
      fTargetComponentsIndex.insert(std::pair<Code, int>(ptarg, k));
      // loop over energies, fNEnBins log. energy bins
      for (int i = 0; i < GetNEnergyBins(); ++i) {
        // hard coded energy grid, has to be aligned to definition in signuc2!!, no
        // comment..
        const units::si::HEPEnergyType Ecm = pow(10., 1. + 1. * i) * 1_GeV;
        // get p-p cross sections
        auto const protonId = Code::Proton;
        auto const [siginel, sigela] =
            fHadronicInteraction.GetCrossSection(protonId, protonId, Ecm);
        const double dsig = siginel / 1_mb;
        const double dsigela = sigela / 1_mb;
        // loop over projectiles, mass numbers from 2 to fMaxNucleusAProjectile
        for (int j = 1; j < gMaxNucleusAProjectile; ++j) {
          const int jj = j + 1;
          double sig_out, dsig_out, sigqe_out, dsigqe_out;
          sigma_mc_(jj, ib, dsig, dsigela, gNSample, sig_out, dsig_out, sigqe_out,
                    dsigqe_out);
          // write to table
          cnucsignuc_.sigma[j][k][i] = sig_out;
          cnucsignuc_.sigqe[j][k][i] = sigqe_out;
        }
      }
    }
    cout << "NuclearInteraction: cross sections for " << fTargetComponentsIndex.size()
         << " components initialized!" << endl;
    for (auto& ptarg : allElementsInUniverse) {
      cout << "cross section table: " << ptarg << endl;
      PrintCrossSectionTable(ptarg);
    }
  }

  template <>
  void NuclearInteraction<SetupEnvironment>::Init() {
    // initialize hadronic interaction module
    // TODO: safe to run multiple initializations?
    if (!fHadronicInteraction.WasInitialized()) fHadronicInteraction.Init();

    // check compatibility of energy ranges, someone could try to use low-energy model..
    if (!fHadronicInteraction.IsValidCoMEnergy(GetMinEnergyPerNucleonCoM()) ||
        !fHadronicInteraction.IsValidCoMEnergy(GetMaxEnergyPerNucleonCoM()))
      throw std::runtime_error(
          "NuclearInteraction: hadronic interaction model incompatible!");

    // initialize nuclib
    // TODO: make sure this does not overlap with sibyll
    nuc_nuc_ini_();

    // initialize cross sections
    InitializeNuclearCrossSections();
  }

  template <>
  units::si::CrossSectionType NuclearInteraction<SetupEnvironment>::ReadCrossSectionTable(
      const int ia, particles::Code pTarget, units::si::HEPEnergyType elabnuc) {
    using namespace corsika::particles;
    using namespace units::si;
    const int ib = fTargetComponentsIndex.at(pTarget) + 1; // table index in fortran
    auto const ECoMNuc = sqrt(2. * corsika::units::constants::nucleonMass * elabnuc);
    if (ECoMNuc < GetMinEnergyPerNucleonCoM() || ECoMNuc > GetMaxEnergyPerNucleonCoM())
      throw std::runtime_error("NuclearInteraction: energy outside tabulated range!");
    const double e0 = elabnuc / 1_GeV;
    double sig;
    cout << "ReadCrossSectionTable: " << ia << " " << ib << " " << e0 << endl;
    signuc2_(ia, ib, e0, sig);
    cout << "ReadCrossSectionTable: sig=" << sig << endl;
    return sig * 1_mb;
  }

  // TODO: remove elastic cross section?
  template <>
  template <>
  tuple<units::si::CrossSectionType, units::si::CrossSectionType>
  NuclearInteraction<SetupEnvironment>::GetCrossSection(Particle& vP,
                                                        const particles::Code TargetId) {
    using namespace units::si;
    if (vP.GetPID() != particles::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: particle not a nucleus!");

    auto const iBeamA = vP.GetNuclearA();
    HEPEnergyType LabEnergyPerNuc = vP.GetEnergy() / iBeamA;
    cout << "NuclearInteraction: GetCrossSection: called with: beamNuclA= " << iBeamA
         << " TargetId= " << TargetId << " LabEnergyPerNuc= " << LabEnergyPerNuc / 1_GeV
         << endl;

    // use nuclib to calc. nuclear cross sections
    // TODO: for now assumes air with hard coded composition
    // extend to arbitrary mixtures, requires smarter initialization
    // get nuclib projectile code: nucleon number
    if (iBeamA > GetMaxNucleusAProjectile() || iBeamA < 2) {
      cout << "NuclearInteraction: beam nucleus outside allowed range for NUCLIB!" << endl
           << "A=" << iBeamA << endl;
      throw std::runtime_error(
          "NuclearInteraction: GetCrossSection: beam nucleus outside allowed range for "
          "NUCLIB!");
    }

    if (fHadronicInteraction.IsValidTarget(TargetId)) {
      auto const sigProd = ReadCrossSectionTable(iBeamA, TargetId, LabEnergyPerNuc);
      cout << "cross section (mb): " << sigProd / 1_mb << endl;
      return std::make_tuple(sigProd, 0_mb);
    } else {
      throw std::runtime_error("target outside range.");
    }
    return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mb,
                           std::numeric_limits<double>::infinity() * 1_mb);
  }

  template <>
  template <>
  units::si::GrammageType NuclearInteraction<SetupEnvironment>::GetInteractionLength(
      Particle& vP) {

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
    corsika::stack::MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.GetEnergy() + constants::nucleonMass;
    int const nuclA = vP.GetNuclearA();
    auto const ElabNuc = vP.GetEnergy() / nuclA;

    corsika::stack::MomentumVector pTotLab(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
    pTotLab += vP.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy
    auto const ECoMNN = sqrt(2. * ElabNuc * constants::nucleonMass);
    cout << "NuclearInteraction: LambdaInt: \n"
         << " input energy: " << Elab / 1_GeV << endl
         << " input energy CoM: " << ECoM / 1_GeV << endl
         << " beam pid:" << corsikaBeamId << endl
         << " beam A: " << nuclA << endl
         << " input energy per nucleon: " << ElabNuc / 1_GeV << endl
         << " input energy CoM per nucleon: " << ECoMNN / 1_GeV << endl;
    //      throw std::runtime_error("stop here");

    // energy limits
    // TODO: values depend on hadronic interaction model !! this is sibyll specific
    if (ElabNuc >= 8.5_GeV && ECoMNN >= gMinEnergyPerNucleonCoM &&
        ECoMNN < gMaxEnergyPerNucleonCoM) {

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
        cout << "NuclearInteraction: get interaction length for target: " << targetId
             << endl;
        auto const [productionCrossSection, elaCrossSection] =
            GetCrossSection(vP, targetId);
        [[maybe_unused]] auto& dummy_elaCrossSection = elaCrossSection;

        cout << "NuclearInteraction: "
             << "IntLength: nuclib return (mb): " << productionCrossSection / 1_mb
             << endl;
        weightedProdCrossSection += w[i] * productionCrossSection;
      }
      cout << "NuclearInteraction: "
           << "IntLength: weighted CrossSection (mb): " << weightedProdCrossSection / 1_mb
           << endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      cout << "NuclearInteraction: "
           << "interaction length (g/cm2): " << int_length * (1_cm * 1_cm / (0.001_kg))
           << endl;

      return int_length;
    } else {
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }
  }

  template <>
  template <>
  process::EProcessReturn NuclearInteraction<SetupEnvironment>::DoInteraction(
      Projectile& vP) {

    // this routine superimposes different nucleon-nucleon interactions
    // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC

    using namespace units;
    using namespace utl;
    using namespace units::si;
    using namespace geometry;

    const auto ProjId = vP.GetPID();
    // TODO: calculate projectile mass in nuclearStackExtension
    //      const auto ProjMass = vP.GetMass();
    cout << "NuclearInteraction: DoInteraction: called with:" << ProjId << endl;

    // check if target-style nucleus (enum)
    if (ProjId != particles::Code::Nucleus)
      throw std::runtime_error(
          "NuclearInteraction: DoInteraction: Wrong nucleus type. Nuclear projectiles "
          "should use NuclearStackExtension!");

    auto const ProjMass =
        vP.GetNuclearZ() * particles::Proton::GetMass() +
        (vP.GetNuclearA() - vP.GetNuclearZ()) * particles::Neutron::GetMass();
    cout << "NuclearInteraction: projectile mass: " << ProjMass / 1_GeV << endl;

    fCount++;

    const CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    // position and time of interaction, not used in NUCLIB
    Point pOrig = vP.GetPosition();
    TimeType tOrig = vP.GetTime();

    cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << endl;
    cout << "Interaction: time: " << tOrig << endl;

    // projectile nucleon number
    const int kAProj = vP.GetNuclearA(); // GetNucleusA(ProjId);
    if (kAProj > GetMaxNucleusAProjectile())
      throw std::runtime_error("Projectile nucleus too large for NUCLIB!");

    // kinematics
    // define projectile nucleus
    HEPEnergyType const eProjectileLab = vP.GetEnergy();
    auto const pProjectileLab = vP.GetMomentum();
    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    cout << "NuclearInteraction: eProj lab: " << eProjectileLab / 1_GeV << endl
         << "NuclearInteraction: pProj lab: " << pProjectileLab.GetComponents() / 1_GeV
         << endl;

    // define projectile nucleon
    HEPEnergyType const eProjectileNucLab = vP.GetEnergy() / kAProj;
    auto const pProjectileNucLab = vP.GetMomentum() / kAProj;
    const FourVector PprojNucLab(eProjectileNucLab, pProjectileNucLab);

    cout << "NuclearInteraction: eProjNucleon lab: " << eProjectileNucLab / 1_GeV << endl
         << "NuclearInteraction: pProjNucleon lab: "
         << pProjectileNucLab.GetComponents() / 1_GeV << endl;

    // define target
    // always a nucleon
    // target is always at rest
    const auto eTargetNucLab = 0_GeV + constants::nucleonMass;
    const auto pTargetNucLab =
        corsika::stack::MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
    const FourVector PtargNucLab(eTargetNucLab, pTargetNucLab);

    cout << "NuclearInteraction: etarget lab: " << eTargetNucLab / 1_GeV << endl
         << "NuclearInteraction: ptarget lab: " << pTargetNucLab.GetComponents() / 1_GeV
         << endl;

    // center-of-mass energy in nucleon-nucleon frame
    auto const PtotNN4 = PtargNucLab + PprojNucLab;
    HEPEnergyType EcmNN = PtotNN4.GetNorm();
    cout << "NuclearInteraction: nuc-nuc cm energy: " << EcmNN / 1_GeV << endl;

    if (!fHadronicInteraction.IsValidCoMEnergy(EcmNN)) {
      cout << "NuclearInteraction: nuc-nuc. CoM energy too low for hadronic "
              "interaction model!"
           << endl;
      throw std::runtime_error("NuclearInteraction: DoInteraction: energy too low!");
    }

    // define boost to NUCLEON-NUCLEON frame
    COMBoost const boost(PprojNucLab, constants::nucleonMass);
    // boost projecticle
    auto const PprojNucCoM = boost.toCoM(PprojNucLab);

    // boost target
    auto const PtargNucCoM = boost.toCoM(PtargNucLab);

    cout << "Interaction: ebeam CoM: " << PprojNucCoM.GetTimeLikeComponent() / 1_GeV
         << endl
         << "Interaction: pbeam CoM: "
         << PprojNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;
    cout << "Interaction: etarget CoM: " << PtargNucCoM.GetTimeLikeComponent() / 1_GeV
         << endl
         << "Interaction: ptarget CoM: "
         << PtargNucCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;

    // sample target nucleon number
    //
    // proton stand-in for nucleon
    const auto beamId = particles::Proton::GetCode();
    auto const* const currentNode = vP.GetNode();
    const auto& mediumComposition =
        currentNode->GetModelProperties().GetNuclearComposition();
    cout << "get nucleon-nucleus cross sections for target materials.." << endl;
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
      cout << "target component: " << targetId << endl;
      cout << "beam id: " << beamId << endl;
      const auto [sigProd, sigEla] =
          fHadronicInteraction.GetCrossSection(beamId, targetId, EcmNN);
      cross_section_of_components[i] = sigProd;
      [[maybe_unused]] auto sigElaCopy = sigEla; // ONLY TO AVOID COMPILER WARNINGS
    }

    const auto targetCode =
        mediumComposition.SampleTarget(cross_section_of_components, fRNG);
    cout << "Interaction: target selected: " << targetCode << endl;
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int kATarget = -1;
    if (IsNucleus(targetCode)) kATarget = GetNucleusA(targetCode);
    if (targetCode == particles::Proton::GetCode()) kATarget = 1;
    cout << "NuclearInteraction: nuclib target code: " << kATarget << endl;
    if (!fHadronicInteraction.IsValidTarget(targetCode))
      throw std::runtime_error("target outside range. ");
    // end of target sampling

    // superposition
    cout << "NuclearInteraction: sampling nuc. multiple interaction structure.. " << endl;
    // get nucleon-nucleon cross section
    // (needed to determine number of nucleon-nucleon scatterings)
    const auto protonId = particles::Proton::GetCode();
    const auto [prodCrossSection, elaCrossSection] =
        fHadronicInteraction.GetCrossSection(protonId, protonId, EcmNN);
    const double sigProd = prodCrossSection / 1_mb;
    const double sigEla = elaCrossSection / 1_mb;
    // sample number of interactions (only input variables, output in common cnucms)
    // nuclear multiple scattering according to glauber (r.i.p.)
    int_nuc_(kATarget, kAProj, sigProd, sigEla);

    cout << "number of nucleons in target           : " << kATarget << endl
         << "number of wounded nucleons in target   : " << cnucms_.na << endl
         << "number of nucleons in projectile       : " << kAProj << endl
         << "number of wounded nucleons in project. : " << cnucms_.nb << endl
         << "number of inel. nuc.-nuc. interactions : " << cnucms_.ni << endl
         << "number of elastic nucleons in target   : " << cnucms_.nael << endl
         << "number of elastic nucleons in project. : " << cnucms_.nbel << endl
         << "impact parameter: " << cnucms_.b << endl;

    // calculate fragmentation
    cout << "calculating nuclear fragments.." << endl;
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
    if (nFragments > GetMaxNFragments())
      throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");

    cout << "number of fragments: " << nFragments << endl;
    for (int j = 0; j < nFragments; ++j)
      cout << "fragment: " << j << " A=" << AFragments[j]
           << " px=" << fragments_.ppp[j][0] << " py=" << fragments_.ppp[j][1]
           << " pz=" << fragments_.ppp[j][2] << endl;

    cout << "adding nuclear fragments to particle stack.." << endl;
    // put nuclear fragments on corsika stack
    for (int j = 0; j < nFragments; ++j) {
      particles::Code specCode;
      const auto nuclA = AFragments[j];
      // get Z from stability line
      const auto nuclZ = int(nuclA / 2.15 + 0.7);

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

      cout << "NuclearInteraction: adding fragment: " << specCode << endl;
      cout << "NuclearInteraction: A,Z: " << nuclA << "," << nuclZ << endl;
      cout << "NuclearInteraction: mass: " << mass / 1_GeV << endl;

      // CORSIKA 7 way
      // spectators inherit momentum from original projectile
      const double mass_ratio = mass / ProjMass;

      cout << "NuclearInteraction: mass ratio " << mass_ratio << endl;

      auto const Plab = PprojLab * mass_ratio;

      cout << "NuclearInteraction: fragment momentum: "
           << Plab.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;

      if (nuclA == 1)
        // add nucleon
        vP.AddSecondary(
            tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
                  geometry::Point, units::si::TimeType>{
                specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
                pOrig, tOrig});
      else
        // add nucleus
        vP.AddSecondary(tuple<particles::Code, units::si::HEPEnergyType,
                              corsika::stack::MomentumVector, geometry::Point,
                              units::si::TimeType, unsigned short, unsigned short>{
            specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
            tOrig, nuclA, nuclZ});
    }

    // add elastic nucleons to corsika stack
    // TODO: the elastic interaction could be external like the inelastic interaction,
    // e.g. use existing ElasticModel
    cout << "adding elastically scattered nucleons to particle stack.." << endl;
    for (int j = 0; j < nElasticNucleons; ++j) {
      // TODO: sample proton or neutron
      auto const elaNucCode = particles::Code::Proton;

      // CORSIKA 7 way
      // elastic nucleons inherit momentum from original projectile
      // neglecting momentum transfer in interaction
      const double mass_ratio = particles::GetMass(elaNucCode) / ProjMass;
      auto const Plab = PprojLab * mass_ratio;

      vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                geometry::Point, units::si::TimeType>{
              elaNucCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
              pOrig, tOrig});
    }

    // add inelastic interactions
    cout << "calculate inelastic nucleon-nucleon interactions.." << endl;
    for (int j = 0; j < nInelNucleons; ++j) {
      // TODO: sample neutron or proton
      auto pCode = particles::Proton::GetCode();
      // temporarily add to stack, will be removed after interaction in DoInteraction
      cout << "inelastic interaction no. " << j << endl;
      auto inelasticNucleon = vP.AddSecondary(
          tuple<particles::Code, units::si::HEPEnergyType, corsika::stack::MomentumVector,
                geometry::Point, units::si::TimeType>{
              pCode, PprojNucLab.GetTimeLikeComponent(),
              PprojNucLab.GetSpaceLikeComponents(), pOrig, tOrig});
      // create inelastic interaction
      cout << "calling HadronicInteraction..." << endl;
      fHadronicInteraction.DoInteraction(inelasticNucleon);
    }

    cout << "NuclearInteraction: DoInteraction: done" << endl;

    return process::EProcessReturn::eOk;
  }

} // namespace corsika::process::sibyll
