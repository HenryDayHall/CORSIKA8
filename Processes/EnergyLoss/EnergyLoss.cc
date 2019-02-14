
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/energy_loss/EnergyLoss.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <cmath>
#include <iostream>
#include <limits>

using namespace std;

using namespace corsika;
using namespace corsika::units::si;
using namespace corsika::setup;
using Particle = Stack::ParticleType;
using Track = Trajectory;

namespace corsika::process::EnergyLoss {

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };

  EnergyLoss::EnergyLoss(MeVgcm2 const vdEdX)
      : fdEdX(vdEdX)
      , fEnergyLossTot(0_GeV) {}

  process::EProcessReturn EnergyLoss::DoContinuous(Particle& p, Track& t, Stack&) {
    GrammageType const dX =
        p.GetNode()->GetModelProperties().IntegratedGrammage(t, t.GetLength());
    HEPEnergyType dE = -dX * fdEdX * pow(p.GetChargeNumber(), 2);
    auto E = p.GetEnergy();
    const auto Ekin = E - p.GetParticleMass();
    auto Enew = E + dE;
    cout << "EnergyLoss " << p.GetPID() << ", z=" << p.GetChargeNumber()
         << ", dX=" << dX / 1_g * square(1_cm) << "g/cm2,  dE=" << dE / 1_MeV << "MeV, "
         << " E=" << E / 1_GeV << "GeV,  Ekin=" << Ekin / 1_GeV
         << ", Enew=" << Enew / 1_GeV << "GeV" << endl;
    auto status = process::EProcessReturn::eOk;
    if (-dE > Ekin) {
      dE = -Ekin;
      Enew = p.GetParticleMass();
      status = process::EProcessReturn::eParticleAbsorbed;
    }
    p.SetEnergy(Enew);
    MomentumUpdate(p, Enew);
    fEnergyLossTot += dE;
    GetXbin(p, dE);
    return status;
  }

  units::si::LengthType EnergyLoss::MaxStepLength(Particle&, Track&) {
    return units::si::meter * std::numeric_limits<double>::infinity();
  }

  void EnergyLoss::MomentumUpdate(corsika::setup::Stack::ParticleType& p,
                                  corsika::units::si::HEPEnergyType Enew) {
    HEPMomentumType Pnew = elab2plab(Enew, p.GetParticleMass());
    auto pnew = p.GetMomentum();
    p.SetMomentum(pnew * Pnew / pnew.GetNorm());
  }

#include <corsika/geometry/CoordinateSystem.h>

  int EnergyLoss::GetXbin(corsika::setup::Stack::ParticleType& p,
                          const HEPEnergyType dE) {

    using namespace corsika::geometry;

    const GrammageType deltaX = 10_g / square(1_cm); // binning

    CoordinateSystem const& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
    Point pos1(rootCS, 0_m, 0_m, 0_m);
    Point pos2(rootCS, 0_m, 0_m, p.GetPosition().GetCoordinates()[2]);
    Vector delta = (pos2 - pos1) / 1_s;
    Trajectory t(Line(pos1, delta), 1_s);

    GrammageType const grammage =
        p.GetNode()->GetModelProperties().IntegratedGrammage(t, t.GetLength());

    const int bin = grammage / deltaX;

    if (!fSave.count(bin)) { cout << "EnergyLoss new x bin " << bin << endl; }
    fSave[bin] += -dE / 1_GeV;
    return bin;
  }

  void EnergyLoss::SaveSave() {

    cout << "EnergyLoss Save " << endl;
    for (auto v : fSave) { cout << v.first << " " << v.second << endl; }
  }

} // namespace corsika::process::EnergyLoss
