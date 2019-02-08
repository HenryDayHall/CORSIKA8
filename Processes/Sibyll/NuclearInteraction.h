
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */


#ifndef _corsika_process_sibyll_nuclearinteraction_h_
#define _corsika_process_sibyll_nuclearinteraction_h_

#include <corsika/process/InteractionProcess.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/sibyll/nuclib.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/COMBoost.h>

namespace corsika::process::sibyll {

  class NuclearInteraction
      : public corsika::process::InteractionProcess<NuclearInteraction> {

    int fCount = 0;
    int fNucCount = 0;

  public:
    NuclearInteraction(corsika::environment::Environment const& env,
                       corsika::process::sibyll::Interaction& hadint)
        : fEnvironment(env)
        , fHadronicInteraction(hadint) {}
    ~NuclearInteraction() {
      std::cout << "Nuclib::NuclearInteraction n=" << fCount << " Nnuc=" << fNucCount
                << std::endl;
    }

    void Init() {

      using corsika::random::RNGManager;
      using std::cout;
      using std::endl;

      // initialize hadronic interaction module
      // TODO: safe to run multiple initializations?
      if (!fHadronicInteraction.WasInitialized()) fHadronicInteraction.Init();

      // initialize nuclib
      // TODO: make sure this does not overlap with sibyll
      nuc_nuc_ini_();
    }

    // TODO: remove number of nucleons, avg target mass is available in environment
    template <typename Particle>
    std::tuple<corsika::units::si::CrossSectionType, corsika::units::si::CrossSectionType>
    GetCrossSection(Particle const& p, const corsika::particles::Code TargetId) {
      using namespace corsika::units::si;
      double sigProd;
      auto const pCode = p.GetPID();
      if (pCode != corsika::particles::Code::Nucleus)
        throw std::runtime_error(
            "NuclearInteraction: GetCrossSection: particle not a nucleus!");

      auto const iBeam = p.GetNuclearA();
      HEPEnergyType LabEnergyPerNuc = p.GetEnergy() / iBeam;
      std::cout << "NuclearInteraction: GetCrossSection: called with: beamNuclA= "
                << iBeam << " TargetId= " << TargetId
                << " LabEnergyPerNuc= " << LabEnergyPerNuc / 1_GeV << std::endl;

      // use nuclib to calc. nuclear cross sections
      // TODO: for now assumes air with hard coded composition
      // extend to arbitrary mixtures, requires smarter initialization
      // get nuclib projectile code: nucleon number
      if (iBeam > 56 || iBeam < 2) {
        std::cout << "NuclearInteraction: beam nucleus outside allowed range for NUCLIB!"
                  << std::endl
                  << "A=" << iBeam << std::endl;
        throw std::runtime_error(
            "NuclearInteraction: GetCrossSection: beam nucleus outside allowed range for "
            "NUCLIB!");
      }

      const double dElabNuc = LabEnergyPerNuc / 1_GeV;
      // TODO: these limitations are still sibyll specific.
      // available target nuclei depends on the hadronic interaction model and the
      // initialization
      if (dElabNuc < 10.)
        throw std::runtime_error("NuclearInteraction: GetCrossSection: energy too low!");

      // TODO: these limitations are still sibyll specific.
      // available target nuclei depends on the hadronic interaction model and the
      // initialization
      if (corsika::particles::IsNucleus(TargetId)) {
        const int iTarget = corsika::particles::GetNucleusA(TargetId);
        if (iTarget > 18 || iTarget == 0)
          throw std::runtime_error(
              "Sibyll target outside range. Only nuclei with A<18 are allowed.");
        std::cout << "NuclearInteraction: calling signuc.." << std::endl;
        std::cout << "WARNING: using hard coded cross section for Nucleus-Air with "
                     "SIBYLL! (fix me!)"
                  << std::endl;
        // TODO: target id is not used because cross section is still hard coded and fixed
        // to air.
        signuc_(iBeam, dElabNuc, sigProd);
        std::cout << "cross section: " << sigProd << std::endl;
        return std::make_tuple(sigProd * 1_mbarn, 0_mbarn);
      }
      return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mbarn,
                             std::numeric_limits<double>::infinity() * 1_mbarn);
    }

    template <typename Particle, typename Track>
    corsika::units::si::GrammageType GetInteractionLength(Particle const& p, Track&) {

      using namespace corsika::units;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      // coordinate system, get global frame of reference
      CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      const particles::Code corsikaBeamId = p.GetPID();
      if (!corsika::particles::IsNucleus(corsikaBeamId)) {
        // no nuclear interaction
        return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
      // check if target-style nucleus (enum)
      if (corsikaBeamId != corsika::particles::Code::Nucleus)
        throw std::runtime_error(
            "NuclearInteraction: GetInteractionLength: Wrong nucleus type. Nuclear "
            "projectiles should use NuclearStackExtension!");

      // read from cross section code table
      const HEPMassType nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                              corsika::particles::Neutron::GetMass());

      // FOR NOW: assume target is at rest
      MomentumVector pTarget(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});

      // total momentum and energy
      HEPEnergyType Elab = p.GetEnergy() + nucleon_mass;
      int const nuclA = p.GetNuclearA();
      auto const ElabNuc = p.GetEnergy() / nuclA;

      MomentumVector pTotLab(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      pTotLab += p.GetMomentum();
      pTotLab += pTarget;
      auto const pTotLabNorm = pTotLab.norm();
      // calculate cm. energy
      const HEPEnergyType ECoM = sqrt(
          (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy
      auto const ECoMNN = sqrt(2. * ElabNuc * nucleon_mass);
      std::cout << "NuclearInteraction: LambdaInt: \n"
                << " input energy: " << Elab / 1_GeV << endl
                << " input energy CoM: " << ECoM / 1_GeV << endl
                << " beam pid:" << corsikaBeamId << endl
                << " beam A: " << nuclA << endl
                << " input energy per nucleon: " << ElabNuc / 1_GeV << endl
                << " input energy CoM per nucleon: " << ECoMNN / 1_GeV << endl;
      //      throw std::runtime_error("stop here");

      // energy limits
      // TODO: values depend on hadronic interaction model !! this is sibyll specific
      if (ElabNuc >= 8.5_GeV && ECoMNN >= 10_GeV) {

        // get target from environment
        /*
          the target should be defined by the Environment,
          ideally as full particle object so that the four momenta
          and the boosts can be defined..
        */
        const auto currentNode =
            fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
        const auto mediumComposition =
            currentNode->GetModelProperties().GetNuclearComposition();
        // determine average interaction length
        // weighted sum
        int i = -1;
        si::CrossSectionType weightedProdCrossSection = 0_mbarn;
        // get weights of components from environment/medium
        const auto w = mediumComposition.GetFractions();
        // loop over components in medium
        for (auto const targetId : mediumComposition.GetComponents()) {
          i++;
          cout << "NuclearInteraction: get interaction length for target: " << targetId
               << endl;
          auto const [productionCrossSection, elaCrossSection] =
              GetCrossSection(p, targetId);
          [[maybe_unused]] auto elaCrossSectionCopy = elaCrossSection;

          std::cout << "NuclearInteraction: "
                    << "IntLength: nuclib return (mb): "
                    << productionCrossSection / 1_mbarn << std::endl;
          weightedProdCrossSection += w[i] * productionCrossSection;
        }
        cout << "NuclearInteraction: "
             << "IntLength: weighted CrossSection (mb): "
             << weightedProdCrossSection / 1_mbarn << endl;

        // calculate interaction length in medium
        GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                        corsika::units::constants::u /
                                        weightedProdCrossSection;
        std::cout << "NuclearInteraction: "
                  << "interaction length (g/cm2): "
                  << int_length / (0.001_kg) * 1_cm * 1_cm << std::endl;

        return int_length;
      } else {
        return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
      }
    }

    template <typename Particle, typename Stack>
    corsika::process::EProcessReturn DoInteraction(Particle& p, Stack& s) {

      // this routine superimposes different nucleon-nucleon interactions
      // in a nucleus-nucleus interaction, based the SIBYLL routine SIBNUC

      using namespace corsika::units;
      using namespace corsika::utl;
      using namespace corsika::units::si;
      using namespace corsika::geometry;
      using std::cout;
      using std::endl;

      const auto ProjId = p.GetPID();
      // TODO: calculate projectile mass in nuclearStackExtension
      //      const auto ProjMass = p.GetMass();
      cout << "NuclearInteraction: DoInteraction: called with:" << ProjId << endl;

      if (!IsNucleus(ProjId)) {
        cout << "WARNING: non nuclear projectile in NUCLIB!" << endl;
        // this should not happen
        // throw std::runtime_error("Non nuclear projectile in NUCLIB!");
        return process::EProcessReturn::eOk;
      }

      // check if target-style nucleus (enum)
      if (ProjId != corsika::particles::Code::Nucleus)
        throw std::runtime_error(
            "NuclearInteraction: DoInteraction: Wrong nucleus type. Nuclear projectiles "
            "should use NuclearStackExtension!");

      auto const ProjMass =
          p.GetNuclearZ() * corsika::particles::Proton::GetMass() +
          (p.GetNuclearA() - p.GetNuclearZ()) * corsika::particles::Neutron::GetMass();
      cout << "NuclearInteraction: projectile mass: " << ProjMass / 1_GeV << endl;

      fCount++;

      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in NUCLIB
      Point pOrig = p.GetPosition();
      TimeType tOrig = p.GetTime();

      cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << endl;
      cout << "Interaction: time: " << tOrig << endl;

      // projectile nucleon number
      const int kAProj = p.GetNuclearA(); // GetNucleusA(ProjId);
      if (kAProj > 56)
        throw std::runtime_error("Projectile nucleus too large for NUCLIB!");

      // kinematics
      // define projectile nucleus
      HEPEnergyType const eProjectileLab = p.GetEnergy();
      auto const pProjectileLab = p.GetMomentum();
      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      cout << "NuclearInteraction: eProj lab: " << eProjectileLab / 1_GeV << endl
           << "NuclearInteraction: pProj lab: " << pProjectileLab.GetComponents() / 1_GeV
           << endl;

      // define projectile nucleon
      HEPEnergyType const eProjectileNucLab = p.GetEnergy() / kAProj;
      auto const pProjectileNucLab = p.GetMomentum() / kAProj;
      const FourVector PprojNucLab(eProjectileNucLab, pProjectileNucLab);

      cout << "NuclearInteraction: eProjNucleon lab: " << eProjectileNucLab / 1_GeV
           << endl
           << "NuclearInteraction: pProjNucleon lab: "
           << pProjectileNucLab.GetComponents() / 1_GeV << endl;

      // define target
      // always a nucleon
      auto constexpr nucleon_mass = 0.5 * (corsika::particles::Proton::GetMass() +
                                           corsika::particles::Neutron::GetMass());
      // target is always at rest
      const auto eTargetNucLab = 0_GeV + nucleon_mass;
      const auto pTargetNucLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargNucLab(eTargetNucLab, pTargetNucLab);

      cout << "NuclearInteraction: etarget lab: " << eTargetNucLab / 1_GeV << endl
           << "NuclearInteraction: ptarget lab: " << pTargetNucLab.GetComponents() / 1_GeV
           << endl;

      // center-of-mass energy in nucleon-nucleon frame
      auto const PtotNN4 = PtargNucLab + PprojNucLab;
      HEPEnergyType EcmNN = PtotNN4.GetNorm();
      cout << "NuclearInteraction: nuc-nuc cm energy: " << EcmNN / 1_GeV << endl;

      if (!fHadronicInteraction.ValidCoMEnergy(EcmNN)) {
        cout << "NuclearInteraction: nuc-nuc. CoM energy too low for hadronic "
                "interaction model!"
             << endl;
        throw std::runtime_error("NuclearInteraction: DoInteraction: energy too low!");
      }

      // define boost to NUCLEON-NUCLEON frame
      COMBoost const boost(PprojNucLab, nucleon_mass);
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
      const auto beamId = corsika::particles::Proton::GetCode();
      const auto currentNode = fEnvironment.GetUniverse()->GetContainingNode(pOrig);
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
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        cout << "target component: " << targetId << endl;
        cout << "beam id: " << beamId << endl;
        const auto [sigProd, sigEla, nNuc] =
            fHadronicInteraction.GetCrossSection(beamId, targetId, EcmNN);
        cross_section_of_components[i] = sigProd;
        [[maybe_unused]] auto sigElaCopy = sigEla; // ONLY TO AVOID COMPILER WARNINGS
        [[maybe_unused]] auto sigNucCopy = nNuc;   // ONLY TO AVOID COMPILER WARNINGS
      }

      const auto targetCode = currentNode->GetModelProperties().SampleTarget(
          cross_section_of_components, fRNG);
      cout << "Interaction: target selected: " << targetCode << endl;
      /*
        FOR NOW: allow nuclei with A<18 or protons only.
        when medium composition becomes more complex, approximations will have to be
        allowed air in atmosphere also contains some Argon.
      */
      int kATarget = -1;
      if (IsNucleus(targetCode)) kATarget = GetNucleusA(targetCode);
      if (targetCode == corsika::particles::Proton::GetCode()) kATarget = 1;
      cout << "NuclearInteraction: nuclib target code: " << kATarget << endl;
      if (kATarget > 18 || kATarget < 1)
        throw std::runtime_error(
            "Sibyll target outside range. Only nuclei with A<18 or protons are "
            "allowed.");
      // end of target sampling

      // superposition
      cout << "NuclearInteraction: sampling nuc. multiple interaction structure.. "
           << endl;
      // get nucleon-nucleon cross section
      // (needed to determine number of nucleon-nucleon scatterings)
      const auto protonId = corsika::particles::Proton::GetCode();
      const auto [prodCrossSection, elaCrossSection, dum] =
          fHadronicInteraction.GetCrossSection(protonId, protonId, EcmNN);
      [[maybe_unused]] auto dumCopy = dum; // ONLY TO AVOID COMPILER WARNING
      const double sigProd = prodCrossSection / 1_mbarn;
      const double sigEla = elaCrossSection / 1_mbarn;
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
      if (nFragments > 60)
        throw std::runtime_error("Number of nuclear fragments in NUCLIB exceeded!");

      cout << "number of fragments: " << nFragments << endl;
      for (int j = 0; j < nFragments; ++j)
        cout << "fragment: " << j << " A=" << AFragments[j]
             << " px=" << fragments_.ppp[j][0] << " py=" << fragments_.ppp[j][1]
             << " pz=" << fragments_.ppp[j][2] << endl;

      cout << "adding nuclear fragments to particle stack.." << endl;
      // put nuclear fragments on corsika stack
      for (int j = 0; j < nFragments; ++j) {
        corsika::particles::Code specCode;
        const auto nuclA = AFragments[j];
        // get Z from stability line
        const auto nuclZ = int(nuclA / 2.15 + 0.7);

        // TODO: do we need to catch single nucleons??
        if (nuclA == 1)
          // TODO: sample neutron or proton
          specCode = corsika::particles::Code::Proton;
        else
          specCode = corsika::particles::Code::Nucleus;

        // TODO: mass of nuclei?
        const HEPMassType mass =
            corsika::particles::Proton::GetMass() * nuclZ +
            (nuclA - nuclZ) *
                corsika::particles::Neutron::GetMass(); // this neglects binding energy

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
          p.AddSecondary(
              std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                         corsika::stack::MomentumVector, corsika::geometry::Point,
                         corsika::units::si::TimeType>{
                  specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
                  pOrig, tOrig});
        else
          // add nucleus
          p.AddSecondary(
              std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                         corsika::stack::MomentumVector, corsika::geometry::Point,
                         corsika::units::si::TimeType, unsigned short, unsigned short>{
                  specCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
                  pOrig, tOrig, nuclA, nuclZ});
      }

      // add elastic nucleons to corsika stack
      // TODO: the elastic interaction could be external like the inelastic interaction,
      // e.g. use existing ElasticModel
      cout << "adding elastically scattered nucleons to particle stack.." << endl;
      for (int j = 0; j < nElasticNucleons; ++j) {
        // TODO: sample proton or neutron
        auto const elaNucCode = corsika::particles::Code::Proton;

        // CORSIKA 7 way
        // elastic nucleons inherit momentum from original projectile
        // neglecting momentum transfer in interaction
        const double mass_ratio = corsika::particles::GetMass(elaNucCode) / ProjMass;
        auto const Plab = PprojLab * mass_ratio;

        p.AddSecondary(
            std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                       corsika::stack::MomentumVector, corsika::geometry::Point,
                       corsika::units::si::TimeType>{
                elaNucCode, Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(),
                pOrig, tOrig});
      }

      // add inelastic interactions
      cout << "calculate inelastic nucleon-nucleon interactions.." << endl;
      for (int j = 0; j < nInelNucleons; ++j) {
        // TODO: sample neutron or proton
        auto pCode = corsika::particles::Proton::GetCode();
        // temporarily add to stack, will be removed after interaction in DoInteraction
        cout << "inelastic interaction no. " << j << endl;
        auto inelasticNucleon = p.AddSecondary(
            std::tuple<corsika::particles::Code, corsika::units::si::HEPEnergyType,
                       corsika::stack::MomentumVector, corsika::geometry::Point,
                       corsika::units::si::TimeType>{
                pCode, PprojNucLab.GetTimeLikeComponent(),
                PprojNucLab.GetSpaceLikeComponents(), pOrig, tOrig});
        // create inelastic interaction
        cout << "calling HadronicInteraction..." << endl;
        fHadronicInteraction.DoInteraction(inelasticNucleon, s);
      }

      // delete parent particle
      p.Delete();

      cout << "NuclearInteraction: DoInteraction: done" << endl;

      return process::EProcessReturn::eOk;
    }

  private:
    corsika::environment::Environment const& fEnvironment;
    corsika::process::sibyll::Interaction& fHadronicInteraction;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("s_rndm");
  };

} // namespace corsika::process::sibyll

#endif
