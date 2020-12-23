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

namespace corsika::pythia8 {

  Interaction::~Interaction() {
    std::cout << "Pythia::Interaction n=" << count_ << std::endl;
  }

  Interaction::Interaction(const bool print_listing)
      : print_listing_(print_listing) {

    std::cout << "Pythia::Interaction n=" << count_ << std::endl;

    // initialize Pythia
    if (!initialized_) {

      pythia_.readString("Print:quiet = off");
      pythia_.readString("Check:particleData = on"); // during init
      pythia_.readString("Check:event = on"); // default: on
      pythia_.readString("Check:levelParticleData = 12"); // 1 is default
      // TODO: proper process initialization for MinBias needed
      pythia_.readString("HardQCD:all = on");
      pythia_.readString("ProcessLevel:resonanceDecays = off");

      pythia_.init();

      // any decays in pythia? if yes need to define which particles
      if (internalDecays_) {
        // define which particles are passed to corsika, i.e. which particles make it into
        // history even very shortlived particles like charm or pi0 are of interest here
        const std::vector<Code> HadronsWeWantTrackedByCorsika = {
            Code::PiPlus,     Code::PiMinus, Code::Pi0,     Code::KMinus,
            Code::KPlus,      Code::K0Long,  Code::K0Short, Code::SigmaPlus,
            Code::SigmaMinus, Code::Lambda0, Code::Xi0,     Code::XiMinus,
            Code::OmegaMinus, Code::DPlus,   Code::DMinus,  Code::D0,
            Code::D0Bar};

        Interaction::setStable(HadronsWeWantTrackedByCorsika);
      }

      // basic initialization of cross section routines
      sigma_.init(&pythia_.info, pythia_.settings, &pythia_.particleData, &pythia_.rndm);

      initialized_ = true;
    }
  }

  void Interaction::setStable(std::vector<Code> const& particleList) {
    for (auto p : particleList) Interaction::setStable(p);
  }

  void Interaction::setUnstable(Code const pCode) {
    std::cout << "Pythia::Interaction: setting " << pCode << " unstable.." << std::endl;
    pythia_.particleData.mayDecay(static_cast<int>(get_PDG(pCode)), true);
  }

  void Interaction::setStable(Code const pCode) {
    std::cout << "Pythia::Interaction: setting " << pCode << " stable.." << std::endl;
    pythia_.particleData.mayDecay(static_cast<int>(get_PDG(pCode)), false);
  }

  void Interaction::configureLabFrameCollision(const Code BeamId, const Code TargetId,
                                               const HEPEnergyType BeamEnergy) {
    // Pythia configuration of the current event
    // very clumsy. I am sure this can be done better..

    // set beam
    // beam id for pythia
    auto const pdgBeam = static_cast<int>(get_PDG(BeamId));
    std::stringstream stBeam;
    stBeam << "Beams:idA = " << pdgBeam;
    pythia_.readString(stBeam.str());
    // set target
    auto pdgTarget = static_cast<int>(get_PDG(TargetId));
    // replace hydrogen with proton, otherwise pythia goes into heavy ion mode!
    if (TargetId == Code::Hydrogen) pdgTarget = static_cast<int>(get_PDG(Code::Proton));
    std::stringstream stTarget;
    stTarget << "Beams:idB = " << pdgTarget;
    pythia_.readString(stTarget.str());
    // set frame to lab. frame
    pythia_.readString("Beams:frameType = 2");
    // set beam energy
    const double Elab = BeamEnergy / 1_GeV;
    std::stringstream stEnergy;
    stEnergy << "Beams:eA = " << Elab;
    pythia_.readString(stEnergy.str());
    // target at rest
    pythia_.readString("Beams:eB = 0.");
    // initialize this config
    pythia_.init();
  }

  bool Interaction::canInteract(Code const pCode) {
    return pCode == Code::Proton || pCode == Code::Neutron || pCode == Code::AntiProton ||
           pCode == Code::AntiNeutron || pCode == Code::PiMinus || pCode == Code::PiPlus;
  }

  std::tuple<CrossSectionType, CrossSectionType> Interaction::getCrossSection(
      const Code BeamId, const Code TargetId, const HEPEnergyType CoMenergy) {
    // interaction possible in pythia?
    if (TargetId == Code::Proton || TargetId == Code::Hydrogen) {
      if (canInteract(BeamId) && isValidCoMEnergy(CoMenergy)) {
        // input particle PDG
        auto const pdgCodeBeam = static_cast<int>(get_PDG(BeamId));
        auto const pdgCodeTarget = static_cast<int>(get_PDG(TargetId));
        const double ecm = CoMenergy / 1_GeV;

        // calculate cross section
        sigma_.calc(pdgCodeBeam, pdgCodeTarget, ecm);
        if (sigma_.hasSigmaTot()) {
          const double sigEla = sigma_.sigmaEl();
          const double sigProd = sigma_.sigmaTot() - sigEla;

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

  //  template <>
  GrammageType Interaction::getInteractionLength(
      corsika::setup::Stack::particle_type const& particle) {

    // coordinate system, get global frame of reference
    MomentumVector const& pMomentum = particle.getMomentum();
    CoordinateSystemPtr const& labCS = pMomentum.getCoordinateSystem();

    Code const corsikaBeamId = particle.getPID();

    // beam particles for pythia : 1, 2, 3 for p, pi, k
    // read from cross section code table
    bool const kInteraction = canInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(labCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = particle.getEnergy() + constants::nucleonMass;
    MomentumVector pTotLab(labCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += pMomentum;
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.getNorm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

    std::cout << "Interaction: LambdaInt: \n"
              << " input energy: " << particle.getEnergy() / 1_GeV << std::endl
              << " beam can interact:" << kInteraction << std::endl
              << " beam pid:" << particle.getPID() << std::endl;

    // TODO: move limits into variables
    if (kInteraction && Elab >= 8.5_GeV && isValidCoMEnergy(ECoM)) {

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */
      const auto* currentNode = particle.getNode();
      const auto mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();
      // determine average interaction length

      auto const weightedProdCrossSection =
          mediumComposition.getWeightedSum([=](auto vTargetID) {
            return std::get<0>(this->getCrossSection(corsikaBeamId, vTargetID, ECoM));
          });

      std::cout << "Interaction: IntLength: weighted CrossSection (mb): "
                << weightedProdCrossSection / 1_mb << std::endl
                << "Interaction: IntLength: average mass number: "
                << mediumComposition.getAverageMassNumber() << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.getAverageMassNumber() *
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

  template <class TView>
  void Interaction::doInteraction(TView& view) {

    auto projectile = view.getProjectile();

    const auto corsikaBeamId = projectile.getPID();
    std::cout << "Pythia::Interaction: "
              << "DoInteraction: " << corsikaBeamId << " interaction? "
              << corsika::pythia8::Interaction::canInteract(corsikaBeamId) << std::endl;

    if (is_nucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by PYTHIA!");
    }

    if (corsika::pythia8::Interaction::canInteract(corsikaBeamId)) {

      // define projectile
      HEPEnergyType const eProjectileLab = projectile.getEnergy();
      auto const pProjectileLab = projectile.getMomentum();
      CoordinateSystemPtr const& labCS = pProjectileLab.getCoordinateSystem();

      // position and time of interaction, not used in Sibyll
      Point pOrig = projectile.getPosition();
      TimeType tOrig = projectile.getTime();

      // define target
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + constants::nucleonMass;
      const auto pTargetLab = MomentumVector(labCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      std::cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << std::endl
                << "Interaction: pbeam lab: " << pProjectileLab.getComponents() / 1_GeV
                << std::endl;
      std::cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << std::endl
                << "Interaction: ptarget lab: " << pTargetLab.getComponents() / 1_GeV
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

      std::cout << "Interaction: ebeam CoM: " << PprojCoM.getTimeLikeComponent() / 1_GeV
                << std::endl
                << "Interaction: pbeam CoM: "
                << PprojCoM.getSpaceLikeComponents().getComponents() / 1_GeV << std::endl;
      std::cout << "Interaction: etarget CoM: " << PtargCoM.getTimeLikeComponent() / 1_GeV
                << std::endl
                << "Interaction: ptarget CoM: "
                << PtargCoM.getSpaceLikeComponents().getComponents() / 1_GeV << std::endl;

      std::cout << "Interaction: position of interaction: " << pOrig.getCoordinates()
                << std::endl;
      std::cout << "Interaction: time: " << tOrig << std::endl;

      HEPEnergyType Etot = eProjectileLab + eTargetLab;
      MomentumVector Ptot = projectile.getMomentum();
      // invariant mass, i.e. cm. energy
      HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.getSquaredNorm());

      // sample target mass number
      const auto* currentNode = projectile.getNode();
      const auto& mediumComposition =
          currentNode->getModelProperties().getNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from getInteractionLength if possible
       */
      //#warning reading interaction cross section again, should not be necessary
      auto const& compVec = mediumComposition.getComponents();
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        const auto [sigProd, sigEla] = getCrossSection(corsikaBeamId, targetId, Ecm);
        [[maybe_unused]] const auto& dummy_sigEla = sigEla;
        cross_section_of_components[i] = sigProd;
      }

      const auto corsikaTargetId =
          mediumComposition.sampleTarget(cross_section_of_components, RNG_);
      std::cout << "Interaction: target selected: " << corsikaTargetId << std::endl;

      if (corsikaTargetId != Code::Hydrogen && corsikaTargetId != Code::Neutron &&
          corsikaTargetId != Code::Proton)
        throw std::runtime_error("DoInteraction: wrong target for PYTHIA");

      std::cout << "Interaction: "
                << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
                << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;

      if (eProjectileLab < 8.5_GeV || !isValidCoMEnergy(Ecm)) {
        std::cout << "Interaction: "
                  << " DoInteraction: should have dropped particle.. "
                  << "THIS IS AN ERROR" << std::endl;
        throw std::runtime_error("energy too low for PYTHIA");

      } else {
        count_++;

        configureLabFrameCollision(corsikaBeamId, corsikaTargetId, eProjectileLab);

        // create event in pytia
        if (!pythia_.next()) throw std::runtime_error("Pythia::DoInteraction: failed!");

        // link to pythia stack
        Pythia8::Event& event = pythia_.event;
        // print final state
        event.list();

        MomentumVector Plab_final(labCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
        HEPEnergyType Elab_final = 0_GeV;
        for (int i = 0; i < event.size(); ++i) {
          Pythia8::Particle& p8p = event[i];
          // skip particles that have decayed in pythia
          if (!p8p.isFinal()) continue;

          auto const pyId = convert_from_PDG(static_cast<PDGCode>(p8p.id()));

          const MomentumVector pyPlab(
              labCS, {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});
          HEPEnergyType const pyEn = p8p.e() * 1_GeV;

          // add to corsika stack
          auto pnew =
              projectile.addSecondary(std::make_tuple(pyId, pyEn, pyPlab, pOrig, tOrig));

          Plab_final += pnew.getMomentum();
          Elab_final += pnew.getEnergy();
        }
        std::cout << "conservation (all GeV): "
                  << "Elab_final=" << Elab_final / 1_GeV
                  << ", Plab_final=" << (Plab_final / 1_GeV).getComponents() << std::endl;
      }
    }
  }

} // namespace corsika::pythia8
