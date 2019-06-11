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

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/process/sibyll/ParticleConversion.h>
#include <corsika/process/sibyll/SibStack.h>
#include <corsika/process/sibyll/sibyll2.3c.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/utl/COMBoost.h>

#include <tuple>

using std::cout;
using std::endl;
using std::tuple;

using namespace corsika;
using namespace corsika::setup;
using SetupParticle = setup::Stack::StackIterator;
using SetupProjectile = setup::StackView::StackIterator;
using Track = Trajectory;

namespace corsika::process::sibyll {

  Interaction::Interaction() {}

  Interaction::~Interaction() {
    cout << "Sibyll::Interaction n=" << fCount << " Nnuc=" << fNucCount << endl;
  }

  void Interaction::Init() {

    using random::RNGManager;

    // initialize Sibyll
    if (!fInitialized) {
      sibyll_ini_();
      fInitialized = true;
    }
  }

  void Interaction::SetStable(std::vector<particles::Code> const& vParticleList) {
    for (auto p : vParticleList) Interaction::SetStable(p);
  }

  void Interaction::SetUnstable(std::vector<particles::Code> const& vParticleList) {
    for (auto p : vParticleList) Interaction::SetUnstable(p);
  }

  void Interaction::SetUnstable(const particles::Code vCode) {
    cout << "Sibyll::Interaction: setting " << vCode << " unstable.." << endl;
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = abs(s_csydec_.idb[s_id - 1]);
  }

  void Interaction::SetStable(const particles::Code vCode) {
    cout << "Sibyll::Interaction: setting " << vCode << " stable.." << endl;
    const int s_id = abs(process::sibyll::ConvertToSibyllRaw(vCode));
    s_csydec_.idb[s_id - 1] = (-1) * abs(s_csydec_.idb[s_id - 1]);
  }

  void Interaction::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
  }

  void Interaction::SetAllUnstable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = abs(s_csydec_.idb[i]);
  }

  tuple<units::si::CrossSectionType, units::si::CrossSectionType>
  Interaction::GetCrossSection(const particles::Code BeamId,
                               const particles::Code TargetId,
                               const units::si::HEPEnergyType CoMenergy) const {
    using namespace units::si;
    double sigProd, sigEla, dummy, dum1, dum3, dum4;
    double dumdif[3];
    const int iBeam = process::sibyll::GetSibyllXSCode(BeamId);
    if (!IsValidCoMEnergy(CoMenergy)) {
      throw std::runtime_error(
          "Interaction: GetCrossSection: CoM energy outside range for Sibyll!");
    }
    const double dEcm = CoMenergy / 1_GeV;
    if (particles::IsNucleus(TargetId)) {
      const int iTarget = particles::GetNucleusA(TargetId);
      if (iTarget > fMaxTargetMassNumber || iTarget == 0)
        throw std::runtime_error(
            "Sibyll target outside range. Only nuclei with A<18 are allowed.");
      sib_sigma_hnuc_(iBeam, iTarget, dEcm, sigProd, dummy, sigEla);
    } else if (TargetId == particles::Proton::GetCode()) {
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
    using namespace geometry;

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    const particles::Code corsikaBeamId = vP.GetPID();

    // beam particles for sibyll : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = process::sibyll::CanInteract(corsikaBeamId);

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

    cout << "Interaction: LambdaInt: \n"
         << " input energy: " << vP.GetEnergy() / 1_GeV << endl
         << " beam can interact:" << kInteraction << endl
         << " beam pid:" << vP.GetPID() << endl;

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
          [=](particles::Code targetID) -> si::CrossSectionType {
            return std::get<0>(this->GetCrossSection(corsikaBeamId, targetID, ECoM));
          });

      cout << "Interaction: "
           << "IntLength: weighted CrossSection (mb): " << weightedProdCrossSection / 1_mb
           << endl;

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      cout << "Interaction: "
           << "interaction length (g/cm2): " << int_length / (0.001_kg) * 1_cm * 1_cm
           << endl;

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function SIBYLL is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  process::EProcessReturn Interaction::DoInteraction(SetupProjectile& vP) {

    using namespace units;
    using namespace utl;
    using namespace units::si;
    using namespace geometry;

    const auto corsikaBeamId = vP.GetPID();
    cout << "ProcessSibyll: "
         << "DoInteraction: " << corsikaBeamId << " interaction? "
         << process::sibyll::CanInteract(corsikaBeamId) << endl;

    if (particles::IsNucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
    }

    if (process::sibyll::CanInteract(corsikaBeamId)) {
      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in Sibyll
      Point pOrig = vP.GetPosition();
      TimeType tOrig = vP.GetTime();

      // define target
      // for Sibyll is always a single nucleon
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + constants::nucleonMass;
      const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      // define projectile
      HEPEnergyType const eProjectileLab = vP.GetEnergy();
      auto const pProjectileLab = vP.GetMomentum();

      cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << endl
           << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
           << endl;
      cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << endl
           << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV << endl;

      const FourVector PprojLab(eProjectileLab, pProjectileLab);

      // define target kinematics in lab frame
      // define boost to and from CoM frame
      // CoM frame definition in Sibyll projectile: +z
      COMBoost const boost(PprojLab, constants::nucleonMass);

      // just for show:
      // boost projecticle
      auto const PprojCoM = boost.toCoM(PprojLab);

      // boost target
      auto const PtargCoM = boost.toCoM(PtargLab);

      cout << "Interaction: ebeam CoM: " << PprojCoM.GetTimeLikeComponent() / 1_GeV
           << endl
           << "Interaction: pbeam CoM: "
           << PprojCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;
      cout << "Interaction: etarget CoM: " << PtargCoM.GetTimeLikeComponent() / 1_GeV
           << endl
           << "Interaction: ptarget CoM: "
           << PtargCoM.GetSpaceLikeComponents().GetComponents() / 1_GeV << endl;

      cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << endl;
      cout << "Interaction: time: " << tOrig << endl;

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
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        const auto [sigProd, sigEla] = GetCrossSection(corsikaBeamId, targetId, Ecm);
        [[maybe_unused]] const auto& dummy_sigEla = sigEla;
        cross_section_of_components[i] = sigProd;
      }

      const auto targetCode =
          mediumComposition.SampleTarget(cross_section_of_components, fRNG);
      cout << "Interaction: target selected: " << targetCode << endl;
      /*
        FOR NOW: allow nuclei with A<18 or protons only.
        when medium composition becomes more complex, approximations will have to be
        allowed air in atmosphere also contains some Argon.
      */
      int targetSibCode = -1;
      if (IsNucleus(targetCode)) targetSibCode = GetNucleusA(targetCode);
      if (targetCode == particles::Proton::GetCode()) targetSibCode = 1;
      cout << "Interaction: sibyll code: " << targetSibCode << endl;
      if (targetSibCode > fMaxTargetMassNumber || targetSibCode < 1)
        throw std::runtime_error(
            "Sibyll target outside range. Only nuclei with A<18 or protons are "
            "allowed.");

      // beam id for sibyll
      const int kBeam = process::sibyll::ConvertToSibyllRaw(corsikaBeamId);

      cout << "Interaction: "
           << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
           << " Ecm(GeV): " << Ecm / 1_GeV << endl;
      if (Ecm > GetMaxEnergyCoM())
        throw std::runtime_error("Interaction::DoInteraction: CoM energy too high!");
      // FR: removed eProjectileLab < 8.5_GeV ||
      if (Ecm < GetMinEnergyCoM()) {
        cout << "Interaction: "
             << " DoInteraction: should have dropped particle.. "
             << "THIS IS AN ERROR" << endl;
        throw std::runtime_error("energy too low for SIBYLL");
      } else {
        fCount++;
        // Sibyll does not know about units..
        const double sqs = Ecm / 1_GeV;
        // running sibyll, filling stack
        sibyll_(kBeam, targetSibCode, sqs);
        if (fInternalDecays) {
          // particles that decay internally will never appear on the corsika stack
          // switch on all decays except for the particles we want to take part in the
          // tracking
          SetAllUnstable();
          SetStable(fTrackedParticles);
          decsib_();
          // reset
          SetAllStable();
        }
        // print final state
        int print_unit = 6;
        sib_list_(print_unit);
        fNucCount += get_nwounded() - 1;

        // add particles from sibyll to stack
        // link to sibyll stack
        SibStack ss;

        MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
        HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
        for (auto& psib : ss) {

          // skip particles that have decayed in Sibyll
          if (psib.HasDecayed()) continue;

          // transform energy to lab. frame
          auto const pCoM = psib.GetMomentum();
          HEPEnergyType const eCoM = psib.GetEnergy();
          auto const Plab = boost.fromCoM(FourVector(eCoM, pCoM));

          // add to corsika stack
          auto pnew = vP.AddSecondary(
              tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
                    geometry::Point, units::si::TimeType>{
                  process::sibyll::ConvertFromSibyll(psib.GetPID()),
                  Plab.GetTimeLikeComponent(), Plab.GetSpaceLikeComponents(), pOrig,
                  tOrig});

          Plab_final += pnew.GetMomentum();
          Elab_final += pnew.GetEnergy();
          Ecm_final += psib.GetEnergy();
        }
        cout << "conservation (all GeV): Ecm_final=" << Ecm_final / 1_GeV << endl
             << "Elab_final=" << Elab_final / 1_GeV
             << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents() << endl;
      }
    }
    return process::EProcessReturn::eOk;
  }

} // namespace corsika::process::sibyll
