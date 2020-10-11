/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
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
#include <corsika/process/sibyll/sibyll2.3d.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/utl/COMBoost.h>

#include <tuple>

using std::make_tuple;
using std::tuple;

using namespace corsika;
using namespace corsika::setup;
using Particle = setup::Stack::StackIterator;
using SetupView = setup::StackView;
using Track = Trajectory;

namespace corsika::process::sibyll {

  bool Interaction::initialized_ = false;

  Interaction::Interaction() {
    using random::RNGManager;

    // initialize Sibyll
    if (!initialized_) {
      sibyll_ini_();
      initialized_ = true;
    }
  }

  Interaction::~Interaction() {
    C8LOG_DEBUG(fmt::format("Sibyll::Interaction n={}, Nnuc={}", count_, nucCount_));
  }

  void Interaction::SetAllStable() {
    for (int i = 0; i < 99; ++i) s_csydec_.idb[i] = -1 * abs(s_csydec_.idb[i]);
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
      if (iTarget > maxTargetMassNumber_ || iTarget == 0)
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
  units::si::GrammageType Interaction::GetInteractionLength(Particle const& vP) const {

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

    C8LOG_DEBUG(
        fmt::format("Interaction: LambdaInt: \n"
                    " input energy: {} GeV "
                    " beam can interact: {} "
                    " beam pid: {}",
                    vP.GetEnergy() / 1_GeV, kInteraction, vP.GetPID()));

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

      C8LOG_DEBUG(
          fmt::format("Interaction: "
                      "IntLength: weighted CrossSection (mb): {} ",
                      weightedProdCrossSection / 1_mb));

      // calculate interaction length in medium
      GrammageType const int_length = mediumComposition.GetAverageMassNumber() *
                                      units::constants::u / weightedProdCrossSection;
      C8LOG_DEBUG(
          fmt::format("Interaction: "
                      "interaction length (g/cm2): {} ",
                      int_length / (0.001_kg) * 1_cm * 1_cm));

      return int_length;
    }

    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  /**
     In this function SIBYLL is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  process::EProcessReturn Interaction::DoInteraction(SetupView& view) {
    using namespace utl;
    using namespace units;
    using namespace units::si;
    using namespace geometry;

    auto const projectile = view.GetProjectile();

    const auto corsikaBeamId = projectile.GetPID();

    if (particles::IsNucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by SIBYLL!");
    }

    // position and time of interaction, not used in Sibyll
    Point const pOrig = projectile.GetPosition();
    TimeType const tOrig = projectile.GetTime();

    // define projectile
    HEPEnergyType const eProjectileLab = projectile.GetEnergy();
    auto const pProjectileLab = projectile.GetMomentum();
    const CoordinateSystem& originalCS = pProjectileLab.GetCoordinateSystem();

    C8LOG_DEBUG(
        "ProcessSibyll: "
        "DoInteraction: pid {} interaction ",
        corsikaBeamId);

    // define target
    // for Sibyll is always a single nucleon
    // FOR NOW: target is always at rest
    const auto eTargetLab = 0_GeV + constants::nucleonMass;
    const auto pTargetLab = MomentumVector(originalCS, 0_GeV, 0_GeV, 0_GeV);
    const FourVector PtargLab(eTargetLab, pTargetLab);

    C8LOG_DEBUG(
        "Interaction: ebeam lab: {} GeV"
        "Interaction: pbeam lab: {} GeV",
        eProjectileLab / 1_GeV, pProjectileLab.GetComponents());
    C8LOG_DEBUG(
        "Interaction: etarget lab: {} GeV "
        "Interaction: ptarget lab: {} GeV",
        eTargetLab / 1_GeV, pTargetLab.GetComponents() / 1_GeV);

    const FourVector PprojLab(eProjectileLab, pProjectileLab);

    // define target kinematics in lab frame
    // define boost to and from CoM frame
    // CoM frame definition in Sibyll projectile: +z
    COMBoost const boost(PprojLab, constants::nucleonMass);
    auto const& csPrime = boost.GetRotatedCS();

    // just for show:
    // boost projecticle
    auto const PprojCoM = boost.toCoM(PprojLab);

    // boost target
    auto const PtargCoM = boost.toCoM(PtargLab);

    C8LOG_DEBUG(
        "Interaction: ebeam CoM: {} GeV "
        "Interaction: pbeam CoM: {} GeV ",
        PprojCoM.GetTimeLikeComponent() / 1_GeV,
        PprojCoM.GetSpaceLikeComponents().GetComponents(csPrime) / 1_GeV);
    C8LOG_DEBUG(
        "Interaction: etarget CoM: {} GeV "
        "Interaction: ptarget CoM: {} GeV ",
        PtargCoM.GetTimeLikeComponent() / 1_GeV,
        PtargCoM.GetSpaceLikeComponents().GetComponents(csPrime) / 1_GeV);

    C8LOG_DEBUG("Interaction: position of interaction: {} ", pOrig.GetCoordinates());
    C8LOG_DEBUG("Interaction: time: {} ", tOrig);

    HEPEnergyType Etot = eProjectileLab + eTargetLab;
    MomentumVector Ptot = projectile.GetMomentum();
    // invariant mass, i.e. cm. energy
    HEPEnergyType Ecm = sqrt(Etot * Etot - Ptot.squaredNorm());

    // sample target mass number
    auto const* currentNode = projectile.GetNode();
    auto const& mediumComposition =
        currentNode->GetModelProperties().GetNuclearComposition();
    // get cross sections for target materials
    /*
      Here we read the cross section from the interaction model again,
      should be passed from GetInteractionLength if possible
     */
    //#warning reading interaction cross section again, should not be necessary
    auto const& compVec = mediumComposition.GetComponents();
    std::vector<CrossSectionType> cross_section_of_components(compVec.size());

    for (size_t i = 0; i < compVec.size(); ++i) {
      auto const targetId = compVec[i];
      const auto [sigProd, sigEla] = GetCrossSection(corsikaBeamId, targetId, Ecm);
      [[maybe_unused]] const auto& dummy_sigEla = sigEla;
      cross_section_of_components[i] = sigProd;
    }

    const auto targetCode =
        mediumComposition.SampleTarget(cross_section_of_components, RNG_);
    C8LOG_DEBUG("Interaction: target selected: {} ", targetCode);
    /*
      FOR NOW: allow nuclei with A<18 or protons only.
      when medium composition becomes more complex, approximations will have to be
      allowed air in atmosphere also contains some Argon.
    */
    int targetSibCode = -1;
    if (IsNucleus(targetCode)) targetSibCode = GetNucleusA(targetCode);
    if (targetCode == particles::Proton::GetCode()) targetSibCode = 1;
    C8LOG_DEBUG("Interaction: sibyll code: {}", targetSibCode);
    if (targetSibCode > maxTargetMassNumber_ || targetSibCode < 1)
      throw std::runtime_error(
          "Sibyll target outside range. Only nuclei with A<18 or protons are "
          "allowed.");

    // beam id for sibyll
    const int kBeam = process::sibyll::ConvertToSibyllRaw(corsikaBeamId);

    C8LOG_DEBUG(
        "Interaction: "
        " DoInteraction: E(GeV): {} "
        " Ecm(GeV): {} ",
        eProjectileLab / 1_GeV, Ecm / 1_GeV);
    if (Ecm > GetMaxEnergyCoM())
      throw std::runtime_error("Interaction::DoInteraction: CoM energy too high!");
    // FR: removed eProjectileLab < 8.5_GeV ||
    if (Ecm < GetMinEnergyCoM()) {
      C8LOG_DEBUG(
          "Interaction: "
          " DoInteraction: should have dropped particle.. "
          "THIS IS AN ERROR");
      throw std::runtime_error("energy too low for SIBYLL");
    } else {
      count_++;
      // Sibyll does not know about units..
      const double sqs = Ecm / 1_GeV;
      // running sibyll, filling stack
      sibyll_(kBeam, targetSibCode, sqs);

      // print final state
      int print_unit = 6;
      sib_list_(print_unit);
      nucCount_ += get_nwounded() - 1;

      // add particles from sibyll to stack
      // link to sibyll stack
      SibStack ss;

      MomentumVector Plab_final(originalCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV, Ecm_final = 0_GeV;
      for (auto& psib : ss) {

        // abort on particles that have decayed in Sibyll. Should not happen!
        if (psib.HasDecayed())
          throw std::runtime_error("found particle that decayed in SIBYLL!");

        // transform 4-momentum to lab. frame
        // note that the momentum needs to be rotated back
        auto const tmp = psib.GetMomentum().GetComponents();
        auto const pCoM = Vector<hepmomentum_d>(csPrime, tmp);
        HEPEnergyType const eCoM = psib.GetEnergy();
        auto const Plab = boost.fromCoM(FourVector(eCoM, pCoM));
        auto const p3lab = Plab.GetSpaceLikeComponents();
        assert(p3lab.GetCoordinateSystem() == originalCS); // just to be sure!

        // add to corsika stack
        auto pnew = view.AddSecondary(
            make_tuple(process::sibyll::ConvertFromSibyll(psib.GetPID()),
                       Plab.GetTimeLikeComponent(), p3lab, pOrig, tOrig));

        Plab_final += pnew.GetMomentum();
        Elab_final += pnew.GetEnergy();
        Ecm_final += psib.GetEnergy();
      }
      C8LOG_DEBUG(
          "conservation (all GeV):"
          "Ecm_initial(per nucleon)={}, Ecm_final(per nucleon)={}, "
          "Elab_initial={}, Elab_final={}, "
          "diff (%)={}, "
          "E in nucleons={}, "
          "Plab_initial={}, "
          "Plab_final={} ",
          Ecm / 1_GeV, Ecm_final * 2. / (get_nwounded() + 1) / 1_GeV, Etot / 1_GeV,
          Elab_final / 1_GeV, (Elab_final / Etot / get_nwounded() - 1) * 100,
          constants::nucleonMass * get_nwounded() / 1_GeV,
          (pProjectileLab / 1_GeV).GetComponents(), (Plab_final / 1_GeV).GetComponents());
    }
    return process::EProcessReturn::eOk;
  }

} // namespace corsika::process::sibyll
