/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/Interaction.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>
#include <corsika/framework/geometry/FourVector.hpp>
#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/modules/sibyll/SibStack.hpp>
#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>
#include <corsika/framework/utility/COMBoost.hpp>

#include <sibyll2.3d.hpp>

#include <tuple>

using std::make_tuple;
using std::tuple;

using namespace corsika;
using SetupParticle = setup::Stack::StackIterator;
using SetupView = setup::StackView;
using Track = setup::Trajectory;

namespace corsika::sibyll {

  Interaction::Interaction() {
    using corsika::RNGManager;

    // initialize Sibyll
    if (!initialized_) {
      sibyll_ini_();
      initialized_ = true;
    }
  }

  Interaction::~Interaction() {
    std::cout << "Sibyll::Interaction n=" << count_ << " Nnuc=" << nucCount_ << std::endl;
  }

  void Interaction::SetStable(std::vector<corsika::Code> const& vParticleList) {
    for (auto p : vParticleList) Interaction::SetStable(p);
  }

  void Interaction::SetUnstable(std::vector<corsika::Code> const& vParticleList) {
    for (auto p : vParticleList) Interaction::SetUnstable(p);
  }

  void Interaction::SetUnstable(const corsika::Code vCode) {
    std::cout << "Sibyll::Interaction: setting " << vCode << " unstable.." << std::endl;
    const int s_id = abs(corsika::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Interaction::SetStable(const corsika::Code vCode) {
    std::cout << "Sibyll::Interaction: setting " << vCode << " stable.." << std::endl;
    const int s_id = abs(corsika::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Interaction::SetAllUnstable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = abs(s_csydec_.idb[i]);
  }

  void Interaction::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
  }

  tuple<units::si::CrossSectionType, units::si::CrossSectionType>
  Interaction::GetCrossSection(const corsika::Code BeamId,
                               const corsika::Code TargetId,
                               const units::si::HEPEnergyType CoMenergy) const {
    using namespace units::si;
    double sigProd, sigEla, dummy, dum1, dum3, dum4;
    double dumdif[3];
    const int iBeam = corsika::sibyll::GetSibyllXSCode(BeamId);
    if (!IsValidCoMEnergy(CoMenergy)) {
      throw std::runtime_error(
          "Interaction: GetCrossSection: CoM energy outside range for Sibyll!");
    }
    const double dEcm = CoMenergy / 1_GeV;
    if (corsika::IsNucleus(TargetId)) {
      const int iTarget = corsika::GetNucleusA(TargetId);
      if (iTarget > maxTargetMassNumber_ || iTarget == 0)
        throw std::runtime_error(
            "Sibyll target outside range. Only nuclei with A<18 are allowed.");
      sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy, sigEla);
    } else if (TargetId == corsika::Proton::GetCode()) {
      sib_sigma_hp_(iBeam, dEcm, dum1, sigEla, sigProd, dumdif, dum3, dum4);
    } else {
      // no interaction in sibyll possible, return infinite cross section? or throw?
      sigProd = std::numeric_limits<double>::infinity();
      sigEla = std::numeric_limits<double>::infinity();
    }
    return std::make_tuple(sigProd * 1_mb, sigEla * 1_mb);
  }


  template <>
  units::si::GrammageType Interaction::GetInteractionLength(
      SetupParticle const& vP) const {

    using namespace units;
    using namespace units::si;

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    const corsika::Code corsikaBeamId = vP.GetPID();

    // beam corsika for sibyll : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = corsika::sibyll::CanInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.GetEnergy() + constants::nucleonMass;
    MomentumVector pTotLab(rootCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += vP.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

    std::cout << "Interaction: LambdaInt: \n"
         << " input energy: " << vP.GetEnergy() / 1_GeV << std::endl
         << " beam can interact:" << kInteraction << std::endl
         << " beam pid:" << vP.GetPID() << std::endl;

    // TODO: move limits into variables
    // FR: removed && Elab >= 8.5_GeV
    if (kInteraction && IsValidCoMEnergy(ECoM)) {

      // get target from environment
      /*
        the target should be defined by the Environment,
        ideally as full particle object so that the four momenta
        and the boosts can be defined..
      */

      auto const* currentNode = vP.GetNode();
      const auto& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();

      si::CrossSectionType weightedProdCrossSection = mediumComposition.WeightedSum(
          [=](corsika::Code targetID) -> si::CrossSectionType {
            return std::get<0>(this->GetCrossSection(corsikaBeamId, targetID, ECoM));
          });

      std::cout << "Interaction: "
           << "IntLength: weighted CrossSection (mb): " << weightedProdCrossSection / 1_mb
           << std::endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      std::cout << "Interaction: "
           << "interaction length (g/cm2): " << int_length / (0.001_kg) * 1_cm * 1_cm
           << std::endl;

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function SIBYLL is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  corsika::EProcessReturn Interaction::DoInteraction(SetupProjectile& vP) {

    using namespace units::si;

    const auto corsikaBeamId = vP.GetPID();
    std::cout << "ProcessSibyll: "
         << "DoInteraction: " << corsikaBeamId << " interaction? "
         << corsika::sibyll::CanInteract(corsikaBeamId) << std::endl;

    if (corsika::IsNucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
    }

    if (corsika::sibyll::CanInteract(corsikaBeamId)) {
      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in Sibyll
      Point pOrig = vP.GetPosition();
      TimeType tOrig = vP.GetTime();

      // define target
      // for Sibyll is always a single nucleon
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + corsika::units::constants::nucleonMass;
      const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      // define projectile
      HEPEnergyType const eProjectileLab = vP.GetEnergy();
      auto const pProjectileLab = vP.GetMomentum();

      std::cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << std::endl
           << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
           << std::endl;
      std::cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << std::endl
           << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV << std::endl;

      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      // define target kinematics in lab frame
      // define boost to and from CoM frame
      // CoM frame definition in Sibyll projectile: +z
      COMBoost const boost(PprojLab, corsika::units::constants::nucleonMass);

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

      std::cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << std::endl;
      std::cout << "Interaction: time: " << tOrig << std::endl;

      HEPEnergyType Etot = eProjectileLab + eTargetLab;
      MomentumVector Ptot = vP.GetMomentum();
      // invariant mass, i.e. cm. energy
      HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());

      // sample target mass number
      auto const* currentNode = vP.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from GetInteractionLength if possible
       */
      //#warning reading interaction cross section again, should not be necessary
      auto const& compVec = mediumComposition.GetComponents();
      std::vector<corsika::units::si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        const auto [sigProd, sigEla] = GetCrossSection(corsikaBeamId, targetId, Ecm);
        [[maybe_unused]] const auto& dummy_sigEla = sigEla;
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.SampleTarget(cross_section_of_components, RNG_);
      std::cout << "Interaction: target selected: " << targetCode << std::endl;
      /*
        FOR NOW: allow nuclei with A<18 or protons only.
        when medium composition becomes more complex, approximations will have to be
        allowed air in atmosphere also contains some Argon.
      */
      int targetSibCode = -1;
      if (IsNucleus(targetCode)) targetSibCode = GetNucleusA(targetCode);
      if (targetCode == corsika::Proton::GetCode()) targetSibCode = 1;
      std::cout << "Interaction: sibyll code: " << targetSibCode << std::endl;
      if (targetSibCode > maxTargetMassNumber_ || targetSibCode < 1)
        throw std::runtime_error(
            "Sibyll target outside range. Only nuclei with A<18 or protons are "
            "allowed.");

      // beam id for sibyll
      const int kBeam = corsika::sibyll::ConvertToSibyllRaw(corsikaBeamId);

      std::cout << "Interaction: "
           << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
           << " Ecm(GeV): " << Ecm / 1_GeV << std::endl;
      if (Ecm > GetMaxEnergyCoM())
        throw std::runtime_error("Interaction::DoInteraction: CoM energy too high!");
      // FR: removed eProjectileLab < 8.5_GeV ||
      if (Ecm < GetMinEnergyCoM()) {
        std::cout << "Interaction: "
             << " DoInteraction: should have dropped particle.. "
             << "THIS IS AN ERROR" << std::endl;
        throw std::runtime_error("energy too low for SIBYLL");
      } else {
        count_++;
        // Sibyll does not know about units..
        const double sqs = Ecm / 1_GeV;
        // running sibyll, filling stack
        std::cout << "kBeam " << kBeam << " targetSibCode " << targetSibCode << " sqs " << sqs << std::endl;
        sibyll_(kBeam, targetSibCode, sqs);
        if (internalDecays_) {
          // corsika that decay internally will never appear on the corsika stack
          // switch on all decays except for the corsika we want to take part in the
          // tracking
          SetAllUnstable();
          SetStable(trackedParticles_);
          decsib_();
          // reset
          SetAllStable();
        }
        // print final state
        int print_unit = 6;
        sib_list_(print_unit);
        nucCount_ += get_nwounded() - 1;

        // add corsika from sibyll to stack
        // link to sibyll stack
        SibStack ss;

        MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
        HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
        for (auto& psib : ss) {

          // skip corsika that have decayed in Sibyll
          if (psib.HasDecayed()) continue;

          // transform energy to lab. frame
          auto const pCoM = psib.GetMomentum();
          HEPEnergyType const eCoM = psib.GetEnergy();
          auto const Plab = boost.fromCoM(FourVector(eCoM, pCoM));

          // add to corsika stack
          auto pnew = vP.AddSecondary(
          std::tuple<corsika::Code, units::si::HEPEnergyType, corsika::MomentumVector,
                    corsika::Point, units::si::TimeType>{
                  corsika::sibyll::ConvertFromSibyll(psib.GetPID()),
                  Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
                  tOrig});

          Plab_final += pnew.GetMomentum();
          Elab_final += pnew.GetEnergy();
          Ecm_final += psib.GetEnergy();
        }
        std::cout << "conservation (all GeV): Ecm_final=" << Ecm_final / 1_GeV << std::endl
             << "Elab_final=" << Elab_final / 1_GeV
             << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents() << std::endl;
      }
    }
    return corsika::EProcessReturn::eOk;
  }

} // namespace corsika::sibyll
