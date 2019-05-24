/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/pythia/Interaction.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/utl/COMBoost.h>

#include <tuple>

using std::cout;
using std::endl;
using std::tuple;

using namespace corsika;
using namespace corsika::setup;
using Projectile = corsika::setup::StackView::ParticleType;
using Particle = corsika::setup::Stack::ParticleType;

namespace corsika::process::pythia {

  typedef corsika::geometry::Vector<corsika::units::si::hepmomentum_d> MomentumVector;

  Interaction::~Interaction() { cout << "Pythia::Interaction n=" << fCount << endl; }

  void Interaction::Init() {

    using random::RNGManager;

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
        const std::vector<particles::Code> HadronsWeWantTrackedByCorsika = {
            particles::Code::PiPlus,     particles::Code::PiMinus,
            particles::Code::Pi0,        particles::Code::KMinus,
            particles::Code::KPlus,      particles::Code::K0Long,
            particles::Code::K0Short,    particles::Code::SigmaPlus,
            particles::Code::SigmaMinus, particles::Code::Lambda0,
            particles::Code::Xi0,        particles::Code::XiMinus,
            particles::Code::OmegaMinus, particles::Code::DPlus,
            particles::Code::DMinus,     particles::Code::D0,
            particles::Code::D0Bar};

        Interaction::SetParticleListStable(HadronsWeWantTrackedByCorsika);
      }

      // basic initialization of cross section routines
      fSigma.init(&fPythia.info, fPythia.settings, &fPythia.particleData, &fPythia.rndm);

      fInitialized = true;
    }
  }

  void Interaction::SetParticleListStable(
      std::vector<particles::Code> const& particleList) {
    for (auto p : particleList) Interaction::SetStable(p);
  }

  void Interaction::SetUnstable(const particles::Code pCode) {
    cout << "Pythia::Interaction: setting " << pCode << " unstable.." << endl;
    fPythia.particleData.mayDecay(static_cast<int>(particles::GetPDG(pCode)), true);
  }

  void Interaction::SetStable(const particles::Code pCode) {
    cout << "Pythia::Interaction: setting " << pCode << " stable.." << endl;
    fPythia.particleData.mayDecay(static_cast<int>(particles::GetPDG(pCode)), false);
  }

  void Interaction::ConfigureLabFrameCollision(
      const particles::Code BeamId, const particles::Code TargetId,
      const units::si::HEPEnergyType BeamEnergy) {
    using namespace units::si;
    // Pythia configuration of the current event
    // very clumsy. I am sure this can be done better..

    // set beam
    // beam id for pythia
    auto const pdgBeam = static_cast<int>(particles::GetPDG(BeamId));
    std::stringstream stBeam;
    stBeam << "Beams:idA = " << pdgBeam;
    fPythia.readString(stBeam.str());
    // set target
    auto pdgTarget = static_cast<int>(particles::GetPDG(TargetId));
    // replace hydrogen with proton, otherwise pythia goes into heavy ion mode!
    if (TargetId == particles::Code::Hydrogen)
      pdgTarget = static_cast<int>(particles::GetPDG(particles::Code::Proton));
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

  bool Interaction::CanInteract(const corsika::particles::Code pCode) {
    return pCode == corsika::particles::Code::Proton ||
           pCode == corsika::particles::Code::Neutron ||
           pCode == corsika::particles::Code::AntiProton ||
           pCode == corsika::particles::Code::AntiNeutron ||
           pCode == corsika::particles::Code::PiMinus ||
           pCode == corsika::particles::Code::PiPlus;
  }

  tuple<units::si::CrossSectionType, units::si::CrossSectionType>
  Interaction::GetCrossSection(const particles::Code BeamId,
                               const particles::Code TargetId,
                               const units::si::HEPEnergyType CoMenergy) {
    using namespace units::si;

    // interaction possible in pythia?
    if (TargetId == particles::Code::Proton || TargetId == particles::Code::Hydrogen) {
      if (CanInteract(BeamId) && ValidCoMEnergy(CoMenergy)) {
        // input particle PDG
        auto const pdgCodeBeam = static_cast<int>(particles::GetPDG(BeamId));
        auto const pdgCodeTarget = static_cast<int>(particles::GetPDG(TargetId));
        const double ecm = CoMenergy / 1_GeV;

        // calculate cross section
        fSigma.calc(pdgCodeBeam, pdgCodeTarget, ecm);
        if (fSigma.hasSigmaTot()) {
          const double sigEla = fSigma.sigmaEl();
          const double sigProd = fSigma.sigmaTot() - sigEla;

          return std::make_tuple(sigProd * 1_mb, sigEla * 1_mb);

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
  units::si::GrammageType Interaction::GetInteractionLength(Particle& p) {

    using namespace units;
    using namespace units::si;
    using namespace geometry;

    // coordinate system, get global frame of reference
    CoordinateSystem& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    const particles::Code corsikaBeamId = p.GetPID();

    // beam particles for pythia : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = CanInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    process::pythia::MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = p.GetEnergy() + constants::nucleonMass;
    process::pythia::MomentumVector pTotLab(rootCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += p.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();
    // calculate cm. energy
    const HEPEnergyType ECoM = sqrt(
        (Elab + pTotLabNorm) * (Elab - pTotLabNorm)); // binomial for numerical accuracy

    cout << "Interaction: LambdaInt: \n"
         << " input energy: " << p.GetEnergy() / 1_GeV << endl
         << " beam can interact:" << kInteraction << endl
         << " beam pid:" << p.GetPID() << endl;

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
        cout << "Interaction: get interaction length for target: " << targetId << endl;

        auto const [productionCrossSection, elaCrossSection] =
            GetCrossSection(corsikaBeamId, targetId, ECoM);
        [[maybe_unused]] const auto& dummy_elaCrossSection = elaCrossSection;

        cout << "Interaction: IntLength: pythia return (mb): "
             << productionCrossSection / 1_mb << endl
             << "Interaction: IntLength: weight : " << w[i] << endl;

        weightedProdCrossSection += w[i] * productionCrossSection;
      }
      cout << "Interaction: IntLength: weighted CrossSection (mb): "
           << weightedProdCrossSection / 1_mb << endl
           << "Interaction: IntLength: average mass number: "
           << mediumComposition.GetAverageMassNumber() << endl;

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
     In this function PYTHIA is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  process::EProcessReturn Interaction::DoInteraction(Projectile& vP) {

    using namespace units;
    using namespace utl;
    using namespace units::si;
    using namespace geometry;

    const auto corsikaBeamId = vP.GetPID();
    cout << "Pythia::Interaction: "
         << "DoInteraction: " << corsikaBeamId << " interaction? "
         << process::pythia::Interaction::CanInteract(corsikaBeamId) << endl;

    if (particles::IsNucleus(corsikaBeamId)) {
      // nuclei handled by different process, this should not happen
      throw std::runtime_error("Nuclear projectile are not handled by PYTHIA!");
    }

    if (process::pythia::Interaction::CanInteract(corsikaBeamId)) {

      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

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

      cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << endl
           << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
           << endl;
      cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << endl
           << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV << endl;

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
      const auto* currentNode = vP.GetNode();
      const auto& mediumComposition =
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

      const auto corsikaTargetId =
          mediumComposition.SampleTarget(cross_section_of_components, fRNG);
      cout << "Interaction: target selected: " << corsikaTargetId << endl;

      if (corsikaTargetId != particles::Code::Hydrogen &&
          corsikaTargetId != particles::Code::Neutron &&
          corsikaTargetId != particles::Code::Proton)
        throw std::runtime_error("DoInteraction: wrong target for PYTHIA");

      cout << "Interaction: "
           << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
           << " Ecm(GeV): " << Ecm / 1_GeV << endl;

      if (eProjectileLab < 8.5_GeV || !ValidCoMEnergy(Ecm)) {
        cout << "Interaction: "
             << " DoInteraction: should have dropped particle.. "
             << "THIS IS AN ERROR" << endl;
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
              particles::ConvertFromPDG(static_cast<particles::PDGCode>(p8p.id()));

          const MomentumVector pyPlab(
              rootCS, {p8p.px() * 1_GeV, p8p.py() * 1_GeV, p8p.pz() * 1_GeV});
          HEPEnergyType const pyEn = p8p.e() * 1_GeV;

          // add to corsika stack
          auto pnew = vP.AddSecondary(
              tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
                    geometry::Point, units::si::TimeType>{pyId, pyEn, pyPlab, pOrig,
                                                          tOrig});

          Plab_final += pnew.GetMomentum();
          Elab_final += pnew.GetEnergy();
        }
        cout << "conservation (all GeV): "
             << "Elab_final=" << Elab_final / 1_GeV
             << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents() << endl;
      }
    }
    return process::EProcessReturn::eOk;
  }

} // namespace corsika::process::pythia
