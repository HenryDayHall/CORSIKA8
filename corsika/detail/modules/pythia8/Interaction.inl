/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/pythia8/Interaction.hpp>

#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/framework/utility/COMBoost.hpp>
#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/setup/SetupStack.hpp>

#include <tuple>

using Projectile = corsika::setup::StackView::ParticleType;
using Particle = corsika::setup::Stack::ParticleType;

namespace corsika::pythia8 {

  typedef corsika::Vector<corsika::hepmomentum_d> MomentumVector;

  Interaction::~Interaction() {
    std::cout << "Pythia::Interaction n=" << fCount << std::endl;
  }

  void Interaction::Init() {

    using corsika::RNGManager;

    // initialize Pythia
    if (!fInitialized) {

      fPythia.readString("Print:quiet = on");
      // TODO: proper process initialization for MinBias needed
      fPythia.readString("HardQCD:all = on");
      fPythia.readString("ProcessLevel:resonanceDecays = off");

      fPythia.init();

      // any decays in pythia? if yes need to define which particles
      if (fInternalDecays) {
        // define which particles are passed to corsika, i.e. which particles make it into
        // history even very shortlived particles like charm or pi0 are of interest here
        const std::vector<corsika::Code> HadronsWeWantTrackedByCorsika = {
            corsika::Code::PiPlus,     corsika::Code::PiMinus,
            corsika::Code::Pi0,        corsika::Code::KMinus,
            corsika::Code::KPlus,      corsika::Code::K0Long,
            corsika::Code::K0Short,    corsika::Code::SigmaPlus,
            corsika::Code::SigmaMinus, corsika::Code::Lambda0,
            corsika::Code::Xi0,        corsika::Code::XiMinus,
            corsika::Code::OmegaMinus, corsika::Code::DPlus,
            corsika::Code::DMinus,     corsika::Code::D0,
            corsika::Code::D0Bar};

        Interaction::SetParticleListStable(HadronsWeWantTrackedByCorsika);
      }

      // basic initialization of cross section routines
      fSigma.init(&fPythia.info, fPythia.settings, &fPythia.particleData, &fPythia.rndm);

      fInitialized = true;
    }
  }

  void Interaction::SetParticleListStable(
      std::vector<corsika::Code> const& particleList) {
    for (auto p : particleList) Interaction::SetStable(p);
  }

  void Interaction::SetUnstable(const corsika::Code pCode) {
    std::cout << "Pythia::Interaction: setting " << pCode << " unstable.." << std::endl;
    fPythia.particleData.mayDecay(static_cast<int>(corsika::get_PDG(pCode)), true);
  }

  void Interaction::SetStable(const corsika::Code pCode) {
    std::cout << "Pythia::Interaction: setting " << pCode << " stable.." << std::endl;
    fPythia.particleData.mayDecay(static_cast<int>(corsika::get_PDG(pCode)), false);
  }

  void Interaction::ConfigureLabFrameCollision(const corsika::Code BeamId,
                                               const corsika::Code TargetId,
                                               const HEPEnergyType BeamEnergy) {
    // Pythia configuration of the current event
    // very clumsy. I am sure this can be done better..

    // set beam
    // beam id for pythia
    auto const pdgBeam = static_cast<int>(corsika::get_PDG(BeamId));
    std::stringstream stBeam;
    stBeam << "Beams:idA = " << pdgBeam;
    fPythia.readString(stBeam.str());
    // set target
    auto pdgTarget = static_cast<int>(corsika::get_PDG(TargetId));
    // replace hydrogen with proton, otherwise pythia goes into heavy ion mode!
    if (TargetId == corsika::Code::Hydrogen)
      pdgTarget = static_cast<int>(corsika::get_PDG(corsika::Code::Proton));
    std::stringstream stTarget;
    stTarget << "Beams:idB = " << pdgTarget;
    fPythia.readString(stTarget.str());
    // set frame to lab. frame
    fPythia.readString("Beams:frameType = 2");
    // set beam energy
    const double Elab = BeamEnergy / 1_GeV;
    std::stringstream stEnergy;
    stEnergy << "Beams:eA = " << Elab;
    fPythia.readString(stEnergy.str());
    // target at rest
    fPythia.readString("Beams:eB = 0.");
    // initialize this config
    fPythia.init();
  }

  bool Interaction::CanInteract(const corsika::Code pCode) {
    return pCode == corsika::Code::Proton || pCode == corsika::Code::Neutron ||
           pCode == corsika::Code::AntiProton || pCode == corsika::Code::AntiNeutron ||
           pCode == corsika::Code::PiMinus || pCode == corsika::Code::PiPlus;
  }

  std::tuple<CrossSectionType, CrossSectionType> Interaction::GetCrossSection(
      const corsika::Code BeamId, const corsika::Code TargetId,
      const HEPEnergyType CoMenergy) {
    // interaction possible in pythia?
    if (TargetId == corsika::Code::Proton || TargetId == corsika::Code::Hydrogen) {
      if (CanInteract(BeamId) && ValidCoMEnergy(CoMenergy)) {
        // input particle PDG
        auto const pdgCodeBeam = static_cast<int>(corsika::get_PDG(BeamId));
        auto const pdgCodeTarget = static_cast<int>(corsika::get_PDG(TargetId));
        const double ecm = CoMenergy / 1_GeV;

        // calculate cross section
        fSigma.calc(pdgCodeBeam, pdgCodeTarget, ecm);
        if (fSigma.hasSigmaTot()) {
          const double sigEla = fSigma.sigmaEl();
          const double sigProd = fSigma.sigmaTot() - sigEla;

          return std::make_tuple(sigProd * (1_fm * 1_fm), sigEla * (1_fm * 1_fm));

        } else
          throw std::runtime_error("pythia cross section init failed");

      } else {
        return std::make_tuple(std::numeric_limits<double>::infinity() * 1_mb,
                               std::numeric_limits<double>::infinity() * 1_mb);
      }
    } else {
      throw std::runtime_error("invalid target for pythia");
    }
  }

  template <>
  GrammageType Interaction::GetInteractionLength(Particle& p) {

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::getInstance().GetRootCoordinateSystem();

    const corsika::Code corsikaBeamId = p.GetPID();

    // beam particles for pythia : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = CanInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    corsika::MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = p.GetEnergy() + constants::nucleonMass;
    corsika::MomentumVector pTotLab(rootCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += p.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

    std::cout << "Interaction: LambdaInt: \n"
              << " input energy: " << p.GetEnergy() / 1_GeV << std::endl
              << " beam can interact:" << kInteraction << std::endl
              << " beam pid:" << p.GetPID() << std::endl;

    // TODO: move limits into variables
    if (kInteraction && Elab >= 8.5_GeV && ValidCoMEnergy(ECoM)) {

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */
      const auto* currentNode = p.GetNode();
      const auto mediumComposition =
          currentNode->GetModelProperties().getNuclearComposition();
      // determine average interaction length

      auto const weightedProdCrossSection =
          mediumComposition.WeightedSum([=](auto vTargetID) {
            return std::get<0>(this->GetCrossSection(corsikaBeamId, vTargetID, ECoM));
          });

      std::cout << "Interaction: IntLength: weighted CrossSection (mb): "
                << weightedProdCrossSection / 1_mb << std::endl
                << "Interaction: IntLength: average mass number: "
                << mediumComposition.GetAverageMassNumber() << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      constants::u / weightedProdCrossSection;
      std::cout << "Interaction: "
                << "interaction length (g/cm2): " << int_length / (0.001_kg) * 1_cm * 1_cm
                << std::endl;

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function PYTHIA is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  void Interaction::doInteraction(Projectile& vP) {

    const auto corsikaBeamId = vP.GetPID();
    std::cout << "Pythia::Interaction: "
              << "DoInteraction: " << corsikaBeamId << " interaction? "
              << corsika::pythia8::Interaction::CanInteract(corsikaBeamId) << std::endl;

    if (corsika::is_nucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by PYTHIA!");
    }

    if (corsika::pythia8::Interaction::CanInteract(corsikaBeamId)) {

      const CoordinateSystem& rootCS =
          RootCoordinateSystem::getInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in Sibyll
      Point pOrig = vP.GetPosition();
      TimeType tOrig = vP.GetTime();

      // define target
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + constants::nucleonMass;
      const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      // define projectile
      HEPEnergyType const eProjectileLab = vP.GetEnergy();
      auto const pProjectileLab = vP.GetMomentum();

      std::cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << std::endl
                << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
                << std::endl;
      std::cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << std::endl
                << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV
                << std::endl;

      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      // define target kinematics in lab frame
      // define boost to and from CoM frame
      // CoM frame definition in Pythia projectile: +z
      COMBoost const boost(PprojLab, constants::nucleonMass);

      // just for show:
      // boost projecticle
      auto const PprojCoM = boost.toCoM(PprojLab);

      // boost target
      auto const PtargCoM = boost.toCoM(PtargLab);

      std::cout << "Interaction: ebeam CoM: " << PprojCoM.GetTimeLikeComponent() / 1_GeV
                << std::endl
                << "Interaction: pbeam CoM: "
                << PprojCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << std::endl;
      std::cout << "Interaction: etarget CoM: " << PtargCoM.GetTimeLikeComponent() / 1_GeV
                << std::endl
                << "Interaction: ptarget CoM: "
                << PtargCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << std::endl;

      std::cout << "Interaction: position of interaction: " << pOrig.GetCoordinates()
                << std::endl;
      std::cout << "Interaction: time: " << tOrig << std::endl;

      HEPEnergyType Etot = eProjectileLab + eTargetLab;
      MomentumVector Ptot = vP.GetMomentum();
      // invariant mass, i.e. cm. energy
      HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());

      // sample target mass number
      const auto* currentNode = vP.GetNode();
      const auto& mediumComposition =
          currentNode->GetModelProperties().getNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from GetInteractionLength if possible
       */
      //#warning reading interaction cross section again, should not be necessary
      auto const& compVec = mediumComposition.GetComponents();
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        const auto [sigProd, sigEla] = GetCrossSection(corsikaBeamId, targetId, Ecm);
        [[maybe_unused]] const auto& dummy_sigEla = sigEla;
        cross_section_of_components[i] = sigProd;
      }

      const auto corsikaTargetId =
          mediumComposition.SampleTarget(cross_section_of_components, fRNG);
      std::cout << "Interaction: target selected: " << corsikaTargetId << std::endl;

      if (corsikaTargetId != corsika::Code::Hydrogen &&
          corsikaTargetId != corsika::Code::Neutron &&
          corsikaTargetId != corsika::Code::Proton)
        throw std::runtime_error("DoInteraction: wrong target for PYTHIA");

      std::cout << "Interaction: "
                << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
                << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;

      if (eProjectileLab < 8.5_GeV || !ValidCoMEnergy(Ecm)) {
        std::cout << "Interaction: "
                  << " DoInteraction: should have dropped particle.. "
                  << "THIS IS AN ERROR" << std::endl;
        throw std::runtime_error("energy too low for PYTHIA");

      } else {
        fCount++;

        ConfigureLabFrameCollision(corsikaBeamId, corsikaTargetId, eProjectileLab);

        // create event in pytia
        if (!fPythia.next()) throw std::runtime_error("Pythia::DoInteraction: failed!");

        // link to pythia stack
        Pythia8::Event& event = fPythia.event;
        // print final state
        event.list();

        MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
        HEPEnergyType Elab_final = 0_GeV;
        for (int i = 0; i < event.size(); ++i) {
          Pythia8::Particle& p8p = event[i];
          // skip particles that have decayed in pythia
          if (!p8p.isFinal()) continue;

          auto const pyId =
              corsika::convert_from_PDG(static_cast<corsika::PDGCode>(p8p.id()));

          const MomentumVector pyPlab(
              rootCS, {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});
          HEPEnergyType const pyEn = p8p.e() * 1_GeV;

          // add to corsika stack
          auto pnew = vP.AddSecondary(
              std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector,
                         corsika::Point, TimeType>{pyId, pyEn, pyPlab, pOrig, tOrig});

          Plab_final += pnew.GetMomentum();
          Elab_final += pnew.GetEnergy();
        }
        std::cout << "conservation (all GeV): "
                  << "Elab_final=" << Elab_final / 1_GeV
                  << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents() << std::endl;
      }
    }
  }

} // namespace corsika::pythia8
