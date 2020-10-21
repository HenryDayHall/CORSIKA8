/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/pythia8/Pythia8.hpp>
#include <corsika/modules/pythia8/Decay.hpp>
#include <corsika/modules/pythia8/Random.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

namespace corsika::pythia8 {

  Decay::Decay(std::vector<corsika::Code> pParticles)
      : fTrackedParticles(pParticles) {}

  Decay::~Decay() { std::cout << "Pythia::Decay n=" << fCount << std::endl; }

  void Decay::Init() {

    Decay::SetParticleListStable(fTrackedParticles);

    // set random number generator in pythia
    Pythia8::RndmEngine* rndm = new corsika::pythia8::Random();
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

  void Decay::SetParticleListStable(const std::vector<corsika::Code> particleList) {
    for (auto p : particleList) Decay::SetStable(p);
  }

  void Decay::SetUnstable(const corsika::Code pCode) {
    std::cout << "Pythia::Decay: setting " << pCode << " unstable.." << std::endl;
    fPythia.particleData.mayDecay(static_cast<int>(corsika::GetPDG(pCode)), true);
  }

  void Decay::SetStable(const corsika::Code pCode) {
    std::cout << "Pythia::Decay: setting " << pCode << " stable.." << std::endl;
    fPythia.particleData.mayDecay(static_cast<int>(corsika::GetPDG(pCode)), false);
  }

  template <typename TParticle>
  TimeType Decay::GetLifetime(TParticle const& p) {

    HEPEnergyType E = p.GetEnergy();
    HEPMassType m = p.GetMass();

    const double gamma = E / m;

    const TimeType t0 = corsika::GetLifetime(p.GetPID());
    auto const lifetime = gamma * t0;

    return lifetime;
  }

  template <typename TProjectile>
  void Decay::DoDecay(TProjectile& vP) {
    using corsika::Point;

    auto const decayPoint = vP.GetPosition();
    auto const t0 = vP.GetTime();

    // coordinate system, get global frame of reference
    corsika::CoordinateSystem& rootCS =
        corsika::RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();

    fCount++;

    // pythia stack
    Pythia8::Event& event = fPythia.event;
    event.reset();

    // set particle unstable
    Decay::SetUnstable(vP.GetPID());

    // input particle PDG
    auto const pdgCode = static_cast<int>(corsika::GetPDG(vP.GetPID()));

    auto const pcomp = vP.GetMomentum().GetComponents();
    double px = pcomp[0] / 1_GeV;
    double py = pcomp[1] / 1_GeV;
    double pz = pcomp[2] / 1_GeV;
    double en = vP.GetEnergy() / 1_GeV;
    double m = corsika::GetMass(vP.GetPID()) / 1_GeV;

    // add particle to pythia stack
    event.append(pdgCode, 1, 0, 0, px, py, pz, en, m);

    if (!fPythia.next())
      std::cout << "Pythia::Decay: decay failed!" << std::endl;
    else
      std::cout << "Pythia::Decay: particles after decay: " << event.size() << std::endl;

    // list final state
    event.list();

    // loop over final state
    for (int i = 0; i < event.size(); ++i)
      if (event[i].isFinal()) {
        auto const pyId =
            corsika::ConvertFromPDG(static_cast<corsika::PDGCode>(event[i].id()));
        HEPEnergyType pyEn = event[i].e() * 1_GeV;
        MomentumVector pyP(rootCS, {event[i].px() * 1_GeV, event[i].py() * 1_GeV,
                                    event[i].pz() * 1_GeV});

        std::cout << "particle: id=" << pyId
                  << " momentum=" << pyP.GetComponents() / 1_GeV << " energy=" << pyEn
                  << std::endl;

        vP.AddSecondary(
            std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector,
                       corsika::Point, TimeType>{pyId, pyEn, pyP, decayPoint, t0});
      }

    // set particle stable
    Decay::SetStable(vP.GetPID());
  }

} // namespace corsika::pythia8
