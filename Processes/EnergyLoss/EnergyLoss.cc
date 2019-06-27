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
using SetupParticle = corsika::setup::Stack::ParticleType;
using SetupTrack = corsika::setup::Trajectory;

namespace corsika::process::energy_loss {

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };

  EnergyLoss::EnergyLoss()
      : fEnergyLossTot(0_GeV)
      , fdX(10_g / square(1_cm)) // profile binning
  {}

  /**
   *   PDG2018, passage of particles through matter
   *
   * Note, that \f$I_{\mathrm{eff}}\f$ of composite media a determined from \f$ \ln I =
   * \sum_i a_i \ln(I_i) \f$ where \f$ a_i \f$ is the fraction of the electron population
   * (\f$\sim Z_i\f$) of the \f$i\f$-th element. This can also be used for shell
   * corrections or density effects.
   *
   * The \f$I_{\mathrm{eff}}\f$ of compounds is not better than a few percent, if not
   * measured explicitly.
   *
   * For shell correction, see Sec 6 of https://www.nap.edu/read/20066/chapter/8#115
   *
   */
  HEPEnergyType EnergyLoss::BetheBloch(SetupParticle const& p, GrammageType const dX) {

    // all these are material constants and have to come through Environment
    // right now: values for nitrogen_D
    // 7 nitrogen_gas 82.0 0.49976 D E 0.0011653 0.0 1.7378 4.1323 0.15349 3.2125 10.54
    auto Ieff = 82.0_eV;
    [[maybe_unused]] auto Zmat = 7;
    auto ZoverA = 0.49976_mol / 1_g;
    const double x0 = 1.7378;
    const double x1 = 4.1323;
    const double Cbar = 10.54;
    const double delta0 = 0.0;
    const double aa = 0.15349;
    const double sk = 3.2125;
    // end of material constants

    // this is the Bethe-Bloch coefficiet 4pi N_A r_e^2 m_e c^2
    auto constexpr K = 0.307075_MeV / 1_mol * square(1_cm);
    HEPEnergyType const E = p.GetEnergy();
    HEPMassType const m = p.GetMass();
    double const gamma = E / m;
    int const Z = p.GetChargeNumber();
    int const Z2 = Z * Z;
    HEPMassType constexpr me = particles::Electron::GetMass();
    auto const m2 = m * m;
    auto constexpr me2 = me * me;
    double const gamma2 = gamma * gamma;

    double const beta2 = (gamma2 - 1) / gamma2; // 1-1/gamma2    (1-1/gamma)*(1+1/gamma);
                                                // (gamma_2-1)/gamma_2 = (1-1/gamma2);
    double constexpr c2 = 1;                    // HEP convention here c=c2=1
    cout << "BetheBloch beta2=" << beta2 << " gamma2=" << gamma2 << endl;
    [[maybe_unused]] double const eta2 = beta2 / (1 - beta2);
    HEPMassType const Wmax =
        2 * me * c2 * beta2 * gamma2 / (1 + 2 * gamma * me / m + me2 / m2);
    // approx, but <<1%    HEPMassType const Wmax = 2*me*c2*beta2*gamma2;      for HEAVY
    // PARTICLES Wmax ~ 2me v2 for non-relativistic particles
    cout << "BetheBloch Wmax=" << Wmax << endl;

    // Sternheimer parameterization, density corrections towards high energies
    // NOTE/TODO: when Cbar is 0 it needs to be approximated from parameterization ->
    // MISSING
    cout << "BetheBloch p.GetMomentum().GetNorm()/m=" << p.GetMomentum().GetNorm() / m
         << endl;
    double const x = log10(p.GetMomentum().GetNorm() / m);
    double delta = 0;
    if (x >= x1) {
      delta = 2 * (log(10)) * x - Cbar;
    } else if (x < x1 && x >= x0) {
      delta = 2 * (log(10)) * x - Cbar + aa * pow((x1 - x), sk);
    } else if (x < x0) { // and IF conductor (otherwise, this is 0)
      delta = delta0 * pow(100, 2 * (x - x0));
    }
    cout << "BetheBloch delta=" << delta << endl;

    // with further low energies correction, accurary ~1% down to beta~0.05 (1MeV for p)

    // shell correction, <~100MeV
    // need more clarity about formulas and units
    const double Cadj = 0;
    /*
    // https://www.nap.edu/read/20066/chapter/8#104
    HEPEnergyType Iadj = 12_eV * Z + 7_eV;  // Iadj<163eV
    if (Iadj>=163_eV)
      Iadj = 9.76_eV * Z + 58.8_eV * pow(Z, -0.19);  // Iadj>=163eV
    double const Cadj = (0.422377/eta2 + 0.0304043/(eta2*eta2) -
    0.00038106/(eta2*eta2*eta2)) * 1e-6 * Iadj*Iadj + (3.858019/eta2 -
    0.1667989/(eta2*eta2) + 0.00157955/(eta2*eta2*eta2)) * 1e-9 * Iadj*Iadj*Iadj;
    */

    // Barkas correction O(Z3) higher-order Born approximation
    // see Appl. Phys. 85 (1999) 1249
    // double A = 1;
    // if (p.GetPID() == particles::Code::Nucleus) A = p.GetNuclearA();
    // double const Erel = (p.GetEnergy()-p.GetMass()) / A / 1_keV;
    // double const Llow = 0.01 * Erel;
    // double const Lhigh = 1.5/pow(Erel, 0.4) + 45000./Zmat * pow(Erel, 1.6);
    // double const barkas = Z * Llow*Lhigh/(Llow+Lhigh); // RU, I think the Z was
    // missing...
    double const barkas = 1; // does not work yet

    // Bloch correction for O(Z4) higher-order Born approximation
    // see Appl. Phys. 85 (1999) 1249
    const double alpha = 1. / 137.035999173;
    double const y2 = Z * Z * alpha * alpha / beta2;
    double const bloch = -y2 * (1.202 - y2 * (1.042 - 0.855 * y2 + 0.343 * y2 * y2));

    // cout << "BetheBloch Erel=" << Erel << " barkas=" << barkas << " bloch=" << bloch <<
    // endl;

    double const aux = 2 * me * c2 * beta2 * gamma2 * Wmax / (Ieff * Ieff);
    return -K * Z2 * ZoverA / beta2 *
           (0.5 * log(aux) - beta2 - Cadj / Z - delta / 2 + barkas + bloch) * dX;
  }

  // radiation losses according to PDG 2018, ch. 33 ref. [5]
  HEPEnergyType EnergyLoss::RadiationLosses(SetupParticle const& vP,
                                            GrammageType const vDX) {
    // simple-minded hard-coded value for b(E) inspired by data from
    // http://pdg.lbl.gov/2018/AtomicNuclearProperties/ for N and O.
    auto constexpr b = 3.0 * 1e-6 * square(1_cm) / 1_g;
    return -vP.GetEnergy() * b * vDX;
  }

  HEPEnergyType EnergyLoss::TotalEnergyLoss(SetupParticle const& vP,
                                            GrammageType const vDX) {
    return BetheBloch(vP, vDX) + RadiationLosses(vP, vDX);
  }

  process::EProcessReturn EnergyLoss::DoContinuous(SetupParticle& p,
                                                   SetupTrack const& t) {
    if (p.GetChargeNumber() == 0) return process::EProcessReturn::eOk;
    GrammageType const dX =
        p.GetNode()->GetModelProperties().IntegratedGrammage(t, t.GetLength());
    cout << "EnergyLoss " << p.GetPID() << ", z=" << p.GetChargeNumber()
         << ", dX=" << dX / 1_g * square(1_cm) << "g/cm2" << endl;
    HEPEnergyType dE = TotalEnergyLoss(p, dX);
    auto E = p.GetEnergy();
    const auto Ekin = E - p.GetMass();
    auto Enew = E + dE;
    cout << "EnergyLoss  dE=" << dE / 1_MeV << "MeV, "
         << " E=" << E / 1_GeV << "GeV,  Ekin=" << Ekin / 1_GeV
         << ", Enew=" << Enew / 1_GeV << "GeV" << endl;
    auto status = process::EProcessReturn::eOk;
    if (-dE > Ekin) {
      dE = -Ekin;
      Enew = p.GetMass();
      status = process::EProcessReturn::eParticleAbsorbed;
    }
    p.SetEnergy(Enew);
    MomentumUpdate(p, Enew);
    fEnergyLossTot += dE;
    GetXbin(p, t, dE);
    return status;
  }

  LengthType EnergyLoss::MaxStepLength(SetupParticle const& vParticle,
                                       SetupTrack const& vTrack) const {
    if (vParticle.GetChargeNumber() == 0) {
      return units::si::meter * std::numeric_limits<double>::infinity();
    }

    auto constexpr dX = 1_g / square(1_cm);
    auto const dE = -TotalEnergyLoss(vParticle, dX); // dE > 0
    //~ auto const Ekin = vParticle.GetEnergy() - vParticle.GetMass();
    auto const maxLoss = 0.01 * vParticle.GetEnergy();
    auto const maxGrammage = maxLoss / dE * dX;

    return vParticle.GetNode()->GetModelProperties().ArclengthFromGrammage(vTrack,
                                                                           maxGrammage) *
           1.0001; // to make sure particle gets absorbed when DoContinuous() is called
  }

  void EnergyLoss::MomentumUpdate(corsika::setup::Stack::ParticleType& vP,
                                  corsika::units::si::HEPEnergyType Enew) {
    HEPMomentumType Pnew = elab2plab(Enew, vP.GetMass());
    auto pnew = vP.GetMomentum();
    vP.SetMomentum(pnew * Pnew / pnew.GetNorm());
  }

#include <corsika/geometry/CoordinateSystem.h>

  int EnergyLoss::GetXbin(SetupParticle const& vP, SetupTrack const& vTrack,
                          const HEPEnergyType dE) {

    using namespace corsika::geometry;

    CoordinateSystem const& rootCS =
        RootCoordinateSystem::GetInstance().GetRootCoordinateSystem();
    Point const pos1(rootCS, 0_m, 0_m, 0_m);
    Point const pos2(rootCS, 0_m, 0_m, vTrack.GetPosition(0).GetCoordinates()[2]);
    auto const delta = (pos2 - pos1) / 1_s;
    Trajectory const t(Line(pos1, delta), 1_s);

    GrammageType const grammage =
        vP.GetNode()->GetModelProperties().IntegratedGrammage(t, t.GetLength());

    const int bin = grammage / fdX;

    // fill longitudinal profile
    if (!fProfile.count(bin)) { cout << "EnergyLoss new x bin " << bin << endl; }
    fProfile[bin] += -dE / 1_GeV;
    return bin;
  }

  void EnergyLoss::PrintProfile() const {

    cout << "EnergyLoss PrintProfile  X-bin [g/cm2]  dE/dX [GeV/g/cm2]  " << endl;
    double const deltaX = fdX / 1_g * square(1_cm);
    for (auto v : fProfile) {
      cout << v.first * deltaX << " " << v.second / deltaX << endl;
    }
  }

} // namespace corsika::process::energy_loss
