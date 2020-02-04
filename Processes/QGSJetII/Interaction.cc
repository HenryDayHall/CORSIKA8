/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/qgsjetII/Interaction.h>

#include <corsika/environment/Environment.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/FourVector.h>
#include <corsika/process/qgsjetII/ParticleConversion.h>
#include <corsika/process/qgsjetII/QGSJetIIFragmentsStack.h>
#include <corsika/process/qgsjetII/QGSJetIIStack.h>
#include <corsika/process/qgsjetII/qgsjet-II-04.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/utl/COMBoost.h>

#include <sstream>
#include <string>
#include <tuple>

using std::cout;
using std::endl;
using std::ostringstream;
using std::string;
using std::tuple;

using namespace corsika;
using namespace corsika::setup;
using SetupParticle = setup::Stack::StackIterator;
using SetupProjectile = setup::StackView::StackIterator;
using Track = Trajectory;

namespace corsika::process::qgsjetII {

  Interaction::Interaction(const string& dataPath)
      : data_path_(dataPath) {
    if (dataPath == "") {
      if (std::getenv("CORSIKA_DATA")) {
        data_path_ = string(std::getenv("CORSIKA_DATA")) + "/QGSJetII/";
        cout << "Searching for QGSJetII data tables in " << data_path_ << endl;
      }
    }
  }

  Interaction::~Interaction() { cout << "QgsjetII::Interaction n=" << count_ << endl; }

  void Interaction::Init() {

    using random::RNGManager;

    // initialize QgsjetII
    if (!initialized_) {
      qgset_();
      datadir DIR(data_path_);
      qgaini_(DIR.data);
      initialized_ = true;
    }
  }

  units::si::CrossSectionType
  Interaction::GetCrossSection(const particles::Code BeamId,
                               const particles::Code TargetId,
                               const units::si::HEPEnergyType Elab,
                               const unsigned int Abeam,
                               const unsigned int Atarget) const {
    using namespace units::si;
    double sigProd = std::numeric_limits<double>::infinity();

    if (process::qgsjetII::CanInteract(BeamId)) {

      const int iBeam = process::qgsjetII::GetQgsjetIIXSCode(BeamId);
      int iTarget = 1;
      if (particles::IsNucleus(TargetId)) {
        iTarget = Atarget;
        if (iTarget > maxMassNumber_ || iTarget <= 0) {
          std::ostringstream txt;
          txt << "QgsjetII target outside range. iTarget=" << iTarget;
          throw std::runtime_error(txt.str().c_str());
        }
      }
      int iProjectile = 1;
      if (particles::IsNucleus(BeamId)) {
        iProjectile = Abeam;
        if (iProjectile > maxMassNumber_ || iProjectile <= 0)
          throw std::runtime_error("QgsjetII target outside range. ");
      }

      cout << "QgsjetII::GetCrossSection Elab=" << Elab << " iBeam=" << iBeam << " iProjectile=" << iProjectile
           << " iTarget=" << iTarget << endl;
      sigProd = qgsect_(Elab/1_GeV, iBeam, iProjectile, iTarget);
      cout << "QgsjetII::GetCrossSection sigProd=" << sigProd << endl;
    }

    return sigProd * 1_mb;
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

    // beam particles for qgsjetII : 1, 2, 3 for p, pi, k
    // read from cross section code table
    const bool kInteraction = process::qgsjetII::CanInteract(corsikaBeamId);

    // FOR NOW: assume target is at rest
    MomentumVector pTarget(rootCS, {0_GeV, 0_GeV, 0_GeV});

    // total momentum and energy
    HEPEnergyType Elab = vP.GetEnergy();
    MomentumVector pTotLab(rootCS, {0_GeV, 0_GeV, 0_GeV});
    pTotLab += vP.GetMomentum();
    pTotLab += pTarget;
    auto const pTotLabNorm = pTotLab.norm();

    cout << "Interaction: LambdaInt: \n"
         << " input energy: " << vP.GetEnergy() / 1_GeV << endl
         << " beam can interact:" << kInteraction << endl
         << " beam pid:" << vP.GetPID() << endl;

    if (kInteraction) {

      int Abeam = 0;
      if (particles::IsNucleus(vP.GetPID())) Abeam = vP.GetNuclearA();

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
            int Atarget = 0;
            if (corsika::particles::IsNucleus(targetID))
              Atarget = particles::GetNucleusA(targetID);
            return GetCrossSection(corsikaBeamId, targetID, Elab, Abeam, Atarget);
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
     In this function QGSJETII is called to produce one event. The
     event is copied (and boosted) into the shower lab frame.
   */

  template <>
  process::EProcessReturn Interaction::DoInteraction(SetupProjectile& vP) {

    using namespace units;
    using namespace utl;
    using namespace units::si;
    using namespace geometry;

    const auto corsikaBeamId = vP.GetPID();
    cout << "ProcessQgsjetII: "
         << "DoInteraction: " << corsikaBeamId << " interaction? "
         << process::qgsjetII::CanInteract(corsikaBeamId) << endl;

    if (process::qgsjetII::CanInteract(corsikaBeamId)) {

      const CoordinateSystem& rootCS =
          RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

      // position and time of interaction, not used in QgsjetII
      Point pOrig = vP.GetPosition();
      TimeType tOrig = vP.GetTime();

      // define target
      // for QgsjetII is always a single nucleon
      // FOR NOW: target is always at rest
      const auto eTargetLab = 0_GeV + constants::nucleonMass;
      const auto pTargetLab = MomentumVector(rootCS, 0_GeV, 0_GeV, 0_GeV);
      const FourVector PtargLab(eTargetLab, pTargetLab);

      // define projectile
      HEPEnergyType const eProjectileLab = vP.GetEnergy();
      auto const pProjectileLab = vP.GetMomentum();

      int Abeam = 0;
      if (particles::IsNucleus(corsikaBeamId)) Abeam = vP.GetNuclearA();

      cout << "Interaction: ebeam lab: " << eProjectileLab / 1_GeV << endl
           << "Interaction: pbeam lab: " << pProjectileLab.GetComponents() / 1_GeV
           << endl;
      cout << "Interaction: etarget lab: " << eTargetLab / 1_GeV << endl
           << "Interaction: ptarget lab: " << pTargetLab.GetComponents() / 1_GeV << endl;

      // artificial momentum, very small momentum, high mass, but direction of primary
      HEPMomentumType pShort = 0.000001_eV;
      const FourVector PprojLab(100_TeV, pProjectileLab * pShort / pProjectileLab.norm());

      // define target kinematics in lab frame
      // define boost to and from CoM frame
      // CoM frame definition in QgsjetII projectile: +z
      COMBoost const boost(PprojLab, vP.GetMass());

      cout << "Interaction: position of interaction: " << pOrig.GetCoordinates() << endl;
      cout << "Interaction: time: " << tOrig << endl;

      // sample target mass number
      auto const* currentNode = vP.GetNode();
      auto const& mediumComposition =
          currentNode->GetModelProperties().GetNuclearComposition();
      // get cross sections for target materials
      /*
        Here we read the cross section from the interaction model again,
        should be passed from GetInteractionLength if possible
       */
      auto const& compVec = mediumComposition.GetComponents();
      std::vector<si::CrossSectionType> cross_section_of_components(compVec.size());

      for (size_t i = 0; i < compVec.size(); ++i) {
        auto const targetId = compVec[i];
        int Atarget = 0;
        if (corsika::particles::IsNucleus(targetId))
          Atarget = particles::GetNucleusA(targetId);
        const auto sigProd =
            GetCrossSection(corsikaBeamId, targetId, eProjectileLab, Abeam, Atarget);
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
      int targetQgsCode = -1;
      if (particles::IsNucleus(targetCode))
        targetQgsCode = particles::GetNucleusA(targetCode);
      if (targetCode == particles::Proton::GetCode()) targetQgsCode = 1;
      cout << "Interaction: target qgsjetII code/A: " << targetQgsCode << endl;
      if (targetQgsCode > maxMassNumber_ || targetQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      int projQgsCode = 1;
      if (particles::IsNucleus(corsikaBeamId)) projQgsCode = vP.GetNuclearA();
      cout << "Interaction: projectile qgsjetII code/A: " << projQgsCode << " "
           << corsikaBeamId << endl;
      if (projQgsCode > maxMassNumber_ || projQgsCode < 1)
        throw std::runtime_error("QgsjetII target outside range.");

      // beam id for qgsjetII
      int kBeam = 2; // default: proton Shouldn't we randomize neutron/proton for nuclei?
      if (corsikaBeamId != particles::Code::Nucleus) {
        kBeam = process::qgsjetII::ConvertToQgsjetIIRaw(corsikaBeamId);
        // from conex
        if (kBeam == 0) { // replace pi0 or rho0 with pi+/pi-
          static int select = 1;
          kBeam = select;
          select *= -1;
        }
        // replace lambda by neutron
        if (kBeam == 6)
          kBeam = 3;
        else if (kBeam == -6)
          kBeam = -3;
        // else if (abs(kBeam)>6) -> throw
      }

      cout << "Interaction: "
           << " DoInteraction: E(GeV):" << eProjectileLab / 1_GeV
	   << endl;
      count_++;
      qgini_(eProjectileLab / 1_GeV, kBeam, projQgsCode, targetQgsCode);
      // this is from CRMC, is this REALLY needed ???
      qgini_(eProjectileLab / 1_GeV, kBeam, projQgsCode, targetQgsCode);
      qgconf_();

      // bookkeeping
      MomentumVector Plab_final(rootCS, {0.0_GeV, 0.0_GeV, 0.0_GeV});
      HEPEnergyType Elab_final = 0_GeV;

      // fragments
      QGSJetIIFragmentsStack qfs;
      for (auto& fragm : qfs) {
	particles::Code idFragm = particles::Code::Nucleus;
	int A = fragm.GetFragmentSize();
	int Z = 0;
	switch (A) {
	case 1: { // proton/neutron
	  idFragm = particles::Code::Proton;
	  MomentumVector Plab_nucleon(rootCS,
				      {0.0_GeV, 0.0_GeV,
					  sqrt((eProjectileLab + particles::Proton::GetMass()) *
					       (eProjectileLab - particles::Proton::GetMass()))});
	  auto const PlabRot = boost.fromCoM(FourVector(eProjectileLab, Plab_nucleon));
	  cout << "secondary fragment>" //<< static_cast<int>(psec.GetPID())
	       << " " << idFragm << " eProjectileLab=" << eProjectileLab << " " << endl;
	  auto pnew = vP.AddSecondary(
				      tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
				      geometry::Point, units::si::TimeType>{
					idFragm, PlabRot.GetTimeLikeComponent(),
					  PlabRot.GetSpaceLikeComponents(), pOrig, tOrig});
	  Plab_final += pnew.GetMomentum();
	  Elab_final += pnew.GetEnergy();
	} break;
	case 2: // deuterium
	  Z = 1;
	  break;
	case 3: // tritium
	  Z = 1;
	  break;
	case 4: // helium
	  Z = 2;
	  break;
            default: // nucleus
	      {
		Z = int(A / 2.15 + 0.7);
            }
          }

          if (idFragm == particles::Code::Nucleus) {
            MomentumVector Plab_nucleus(rootCS,
                                        {0.0_GeV, 0.0_GeV,
                                         sqrt((eProjectileLab + constants::nucleonMass * A) *
                                              (eProjectileLab - constants::nucleonMass * A))});
            auto const PlabRot = boost.fromCoM(FourVector(eProjectileLab * A, Plab_nucleus));
            cout << "secondary fragment>" 
                 << " " << idFragm << " eProjectileLab=" << eProjectileLab << " A=" << A << " Z=" << Z
                 << endl;
            auto pnew = vP.AddSecondary(
                tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
                      geometry::Point, units::si::TimeType, unsigned short,
                      unsigned short>{idFragm, PlabRot.GetTimeLikeComponent(),
                                      PlabRot.GetSpaceLikeComponents(), pOrig, tOrig, A,
                                      Z});
            Plab_final += pnew.GetMomentum();
            Elab_final += pnew.GetEnergy();
          }
        }

        // secondaries
        QGSJetIIStack qs;
        for (auto& psec : qs) {

          auto const Plab = psec.GetMomentum();
          auto const Elab = psec.GetEnergy();

          // transform energy to lab. frame
          auto const PlabRot = boost.fromCoM(FourVector(Elab, Plab));

          cout << "secondary hadron>" //<< static_cast<int>(psec.GetPID())
               << " " << process::qgsjetII::ConvertFromQgsjetII(psec.GetPID())
               << " Elab=" << Elab << " " << endl;

          // add to corsika stack
          auto pnew = vP.AddSecondary(
              tuple<particles::Code, units::si::HEPEnergyType, stack::MomentumVector,
                    geometry::Point, units::si::TimeType>{
                  process::qgsjetII::ConvertFromQgsjetII(psec.GetPID()),
                  PlabRot.GetTimeLikeComponent(), PlabRot.GetSpaceLikeComponents(),
                  pOrig, tOrig});

          Plab_final += pnew.GetMomentum();
          Elab_final += pnew.GetEnergy();
        }
        cout << "conservation (all GeV): Ecm_final= n/a" /* << Ecm_final / 1_GeV*/ << endl
             << "Elab_final=" << Elab_final / 1_GeV
             << ", Plab_final=" << (Plab_final / 1_GeV).GetComponents()
             << ", N_wounded,targ="
             << QGSJetIIFragmentsStackData::GetWoundedNucleonsTarget()
             << ", N_wounded,proj="
             << QGSJetIIFragmentsStackData::GetWoundedNucleonsProjectile()
             << ", N_fragm,proj=" << qfs.GetSize() << endl;
    }
    return process::EProcessReturn::eOk;
  }
  
} // namespace corsika::process::qgsjetII
