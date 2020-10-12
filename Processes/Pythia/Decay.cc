/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <Pythia8/Pythia.h>
#include <corsika/process/pythia/Decay.h>
#include <corsika/process/pythia/Random.h>

#include <corsika/geometry/FourVector.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/utl/COMBoost.h>

using std::cout;
using std::endl;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika::setup;
using View = corsika::setup::StackView;
using Particle = corsika::setup::Stack::ParticleType;
using Track = Trajectory;

namespace corsika::process::pythia {

  Decay::Decay(const bool print_listing)
      : print_listing_(print_listing) {

    // set random number generator in pythia
    Pythia8::RndmEngine* rndm = new corsika::process::pythia::Random();
    fPythia.setRndmEnginePtr(rndm);

    /*
       issue xyz: definition of particles and decay channels use the same mechanism in
       corsika and pythia we should force pythia to use the file in corsika.
     */
    // bool ParticleData::reInit(string startFile, bool xmlFormat = true)
    // read in particle data from Corsika 8
    // fPythia.particleData.reInit("/home/felix/ngcorsika/corsika-build/include/corsika/particles/ParticleData.xml");
    // fPythia.particleData.checkTable();

    fPythia.readString("Next:numberShowInfo = 0");
    fPythia.readString("Next:numberShowProcess = 0");
    fPythia.readString("Next:numberShowEvent = 0");

    fPythia.readString("Print:quiet = on");
    fPythia.readString("Check:particleData = 0");

    /*
       switching off event check in pythia is needed to allow decays that are off-shell
       according to the mass definition in pythia.
       the consistency of particle masses between event generators is an unsolved issues
    */
    cout << "Pythia::Init: switching off event checking in pythia.." << endl;
    fPythia.readString("Check:event = 1");

    fPythia.readString("ProcessLevel:all = off");
    fPythia.readString("ProcessLevel:resonanceDecays = off");

    // making sure
    SetStable(particles::Code::Pi0);

    //    fPythia.particleData.readString("59:m0 = 101.00");

    if (!fPythia.init())
      throw std::runtime_error("Pythia::Decay: Initialization failed!");
  }

  Decay::Decay(std::set<particles::Code> vHandled)
      : handleAllDecays_(false)
      , handledDecays_(vHandled) {}

  Decay::~Decay() { cout << "Pythia::Decay n=" << fCount << endl; }

  bool Decay::CanHandleDecay(const particles::Code vParticleCode) {
    using namespace corsika::particles;
    // if known to pythia and not proton, electron or neutrino it can decay
    if (vParticleCode == Code::Proton || vParticleCode == Code::AntiProton ||
        vParticleCode == Code::NuE || vParticleCode == Code::NuMu ||
        vParticleCode == Code::NuTau || vParticleCode == Code::NuEBar ||
        vParticleCode == Code::NuMuBar || vParticleCode == Code::NuTauBar ||
        vParticleCode == Code::Electron || vParticleCode == Code::Positron)
      return false;
    else if (CanDecay(vParticleCode)) // non-zero for particles known to sibyll
      return true;
    else
      return false;
  }

  void Decay::SetHandleDecay(const particles::Code vParticleCode) {
    handleAllDecays_ = false;
    cout << "Pythia::Decay: set to handle decay of " << vParticleCode << endl;
    if (Decay::CanHandleDecay(vParticleCode))
      handledDecays_.insert(vParticleCode);
    else
      throw std::runtime_error("this decay can not be handled by pythia!");
  }

  void Decay::SetHandleDecay(const vector<particles::Code> vParticleList) {
    handleAllDecays_ = false;
    for (auto p : vParticleList) SetHandleDecay(p);
  }

  bool Decay::IsDecayHandled(const corsika::particles::Code vParticleCode) {
    if (handleAllDecays_ && CanHandleDecay(vParticleCode))
      return true;
    else
      return handledDecays_.find(vParticleCode) != Decay::handledDecays_.end();
  }

  bool Decay::IsStable(const particles::Code vCode) {
    return fPythia.particleData.canDecay(static_cast<int>(particles::GetPDG(vCode)));
  }

  void Decay::PrintDecayConfig(const particles::Code vCode) {
    cout << "Decay: Pythia decay configuration:" << endl;
    cout << vCode << " is ";
    if (IsStable(vCode))
      cout << "stable" << endl;
    else
      cout << "unstable" << endl;
  }

  void Decay::PrintDecayConfig() {
    cout << "Pythia::Decay: decay configuration:" << endl;
    if (handleAllDecays_)
      cout << " all particles known to Pythia are handled by Pythia::Decay!" << endl;
    else
      for (auto& pCode : handledDecays_)
        cout << "Decay of " << pCode << " is handled by Pythia!" << endl;
  }

  void Decay::SetStable(const vector<particles::Code> particleList) {
    for (auto p : particleList) Decay::SetStable(p);
  }

  bool Decay::CanDecay(const particles::Code pCode) {
    std::cout << "Pythia::Decay: checking if particle: " << pCode
              << " can decay in PYTHIA? ";
    const bool ans =
        fPythia.particleData.canDecay(static_cast<int>(particles::GetPDG(pCode)));
    std::cout << ans << std::endl;
    return ans;
  }

  void Decay::SetUnstable(const particles::Code pCode) {
    cout << "Pythia::Decay: setting " << pCode << " unstable.." << endl;
    fPythia.particleData.mayDecay(static_cast<int>(particles::GetPDG(pCode)), true);
  }

  void Decay::SetStable(const particles::Code pCode) {
    cout << "Pythia::Decay: setting " << pCode << " stable.." << endl;
    fPythia.particleData.mayDecay(static_cast<int>(particles::GetPDG(pCode)), false);
  }

  template <>
  units::si::TimeType Decay::GetLifetime(Particle const& vP) {
    using namespace units::si;

    const auto pid = vP.GetPID();
    if (CanDecay(pid)) {
      HEPEnergyType E = vP.GetEnergy();
      HEPMassType m = vP.GetMass();

      const double gamma = E / m;

      const TimeType t0 = particles::GetLifetime(pid);
      auto const lifetime = gamma * t0;
      cout << "Pythia::Decay: code: " << vP.GetPID() << endl;
      cout << "Pythia::Decay: MinStep: t0: " << t0 << endl;
      cout << "Pythia::Decay: MinStep: energy: " << E / 1_GeV << " GeV" << endl;
      cout << "Pythia::Decay: momentum: " << vP.GetMomentum().GetComponents() / 1_GeV
           << " GeV" << endl;
      cout << "Pythia::Decay: MinStep: gamma: " << gamma << endl;
      cout << "Pythia::Decay: MinStep: tau: " << lifetime << endl;

      return lifetime;
    } else
      return std::numeric_limits<double>::infinity() * 1_s;
  }

  template <>
  void Decay::DoDecay(View& view) {
    using geometry::Point;
    using namespace units;
    using namespace units::si;

    auto const projectile = view.GetProjectile();

    auto const& decayPoint = projectile.GetPosition();
    auto const t0 = projectile.GetTime();

    auto const& labMomentum = projectile.GetMomentum();
    geometry::CoordinateSystem const& labCS = labMomentum.GetCoordinateSystem();

    // define target kinematics in lab frame
    // define boost to and from CoM frame
    // CoM frame definition in Pythia projectile: +z
    utl::COMBoost const boost(labMomentum, projectile.GetMass());
    auto const& rotatedCS = boost.GetRotatedCS();

    fCount++;

    // pythia stack
    Pythia8::Event& event = fPythia.event;
    event.reset();

    auto const particleId = projectile.GetPID();

    // set particle unstable
    Decay::SetUnstable(particleId);

    // input particle PDG
    auto const pdgCode = static_cast<int>(particles::GetPDG(particleId));

    double constexpr px = 0;
    double constexpr py = 0;
    double constexpr pz = 0;
    double const en = projectile.GetMass() / 1_GeV;
    double const m = en;

    // add particle to pythia stack
    event.append(pdgCode, 1, 0, 0, px, py, pz, en, m);

    if (!fPythia.next())
      throw std::runtime_error("Pythia::Decay: decay failed!");
    else
      cout << "Pythia::Decay: particles after decay: " << event.size() << endl;

    if (print_listing_) {
      // list final state
      event.list();
    }

    // loop over final state
    for (int i = 0; i < event.size(); ++i)
      if (event[i].isFinal()) {
        auto const pyId =
            particles::ConvertFromPDG(static_cast<particles::PDGCode>(event[i].id()));
        HEPEnergyType const Erest = event[i].e() * 1_GeV;
        MomentumVector const pRest(
            rotatedCS,
            {event[i].px() * 1_GeV, event[i].py() * 1_GeV, event[i].pz() * 1_GeV});
        geometry::FourVector const fourMomRest{Erest, pRest};
        auto const fourMomLab = boost.fromCoM(fourMomRest);

        cout << "particle: id=" << pyId << " momentum="
             << fourMomLab.GetSpaceLikeComponents().GetComponents(labCS) / 1_GeV
             << " energy=" << fourMomLab.GetTimeLikeComponent() << endl;

        view.AddSecondary(
            tuple<particles::Code, units::si::HEPEnergyType,
                  corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
                pyId, fourMomLab.GetTimeLikeComponent(),
                fourMomLab.GetSpaceLikeComponents(), decayPoint, t0});
      }

    // set particle stable
    Decay::SetStable(particleId);
  }

} // namespace corsika::process::pythia
