/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <Pythia8/Pythia.h>
#include <corsika/process/pythia/Decay.h>
#include <corsika/process/pythia/Random.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using std::cout;
using std::endl;
using std::tuple;
using std::vector;

using namespace corsika;
using namespace corsika::setup;
using Projectile = corsika::setup::StackView::ParticleType;
using Particle = corsika::setup::Stack::ParticleType;
using Track = Trajectory;

namespace corsika::process::pythia {

  Decay::Decay(vector<particles::Code> pParticles)
      : fTrackedParticles(pParticles) {}

  Decay::~Decay() { cout << "Pythia::Decay n=" << fCount << endl; }

  void Decay::Init() {

    Decay::SetParticleListStable(fTrackedParticles);

    // set random number generator in pythia
    Pythia8::RndmEngine* rndm = new corsika::process::pythia::Random();
    fPythia.setRndmEnginePtr(rndm);

    fPythia.readString("Next:numberShowInfo = 0");
    fPythia.readString("Next:numberShowProcess = 0");
    fPythia.readString("Next:numberShowEvent = 0");

    fPythia.readString("Print:quiet = on");

    fPythia.readString("ProcessLevel:all = off");
    fPythia.readString("ProcessLevel:resonanceDecays = off");

    fPythia.particleData.readString("59:m0 = 101.00");

    fPythia.init();
  }

  void Decay::SetParticleListStable(const vector<particles::Code> particleList) {
    for (auto p : particleList) Decay::SetStable(p);
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
  units::si::TimeType Decay::GetLifetime(Particle const& p) {
    using namespace units::si;

    HEPEnergyType E = p.GetEnergy();
    HEPMassType m = particles::GetMass(p.GetPID());

    const double gamma = E / m;

    const TimeType t0 = particles::GetLifetime(p.GetPID());
    auto const lifetime = gamma * t0;

    return lifetime;
  }

  template <>
  void Decay::DoDecay(Projectile& vP) {
    using geometry::Point;
    using namespace units;
    using namespace units::si;

    auto const decayPoint = vP.GetPosition();
    auto const t0 = vP.GetTime();

    // coordinate system, get global frame of reference
    geometry::CoordinateSystem& rootCS =
        geometry::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    fCount++;

    // pythia stack
    Pythia8::Event& event = fPythia.event;
    event.reset();

    // set particle unstable
    Decay::SetUnstable(vP.GetPID());

    // input particle PDG
    auto const pdgCode = static_cast<int>(particles::GetPDG(vP.GetPID()));

    auto const pcomp = vP.GetMomentum().GetComponents();
    double px = pcomp[0] / 1_GeV;
    double py = pcomp[1] / 1_GeV;
    double pz = pcomp[2] / 1_GeV;
    double en = vP.GetEnergy() / 1_GeV;
    double m = particles::GetMass(vP.GetPID()) / 1_GeV;

    // add particle to pythia stack
    event.append(pdgCode, 1, 0, 0, px, py, pz, en, m);

    if (!fPythia.next())
      cout << "Pythia::Decay: decay failed!" << endl;
    else
      cout << "Pythia::Decay: particles after decay: " << event.size() << endl;

    // list final state
    event.list();

    // loop over final state
    for (int i = 0; i < event.size(); ++i)
      if (event[i].isFinal()) {
        auto const pyId =
            particles::ConvertFromPDG(static_cast<particles::PDGCode>(event[i].id()));
        HEPEnergyType pyEn = event[i].e() * 1_GeV;
        MomentumVector pyP(rootCS, {event[i].px() * 1_GeV, event[i].py() * 1_GeV,
                                    event[i].pz() * 1_GeV});

        cout << "particle: id=" << pyId << " momentum=" << pyP.GetComponents() / 1_GeV
             << " energy=" << pyEn << endl;

        vP.AddSecondary(
            tuple<particles::Code, units::si::HEPEnergyType,
                  corsika::stack::MomentumVector, geometry::Point, units::si::TimeType>{
                pyId, pyEn, pyP, decayPoint, t0});
      }

    // set particle stable
    Decay::SetStable(vP.GetPID());
  }

} // namespace corsika::process::pythia
