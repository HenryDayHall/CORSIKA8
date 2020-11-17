/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/energy_loss/BetheBlochPDG.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <corsika/framework/geometry/Line.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

using SetupParticle = corsika::setup::Stack::ParticleType;
using SetupTrack = corsika::setup::Trajectory;

namespace corsika::energy_loss {

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };

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
  HEPEnergyType BetheBlochPDG::BetheBloch(SetupParticle const& p, GrammageType const dX) {

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
    HEPMassType constexpr me = corsika::Electron::mass();
    auto const m2 = m * m;
    auto constexpr me2 = me * me;
    double const gamma2 = gamma * gamma;

    double const beta2 = (gamma2 - 1) / gamma2; // 1-1/gamma2    (1-1/gamma)*(1+1/gamma);
                                                // (gamma_2-1)/gamma_2 = (1-1/gamma2);
    double constexpr c2 = 1;                    // HEP convention here c=c2=1
    std::cout << "BetheBloch beta2=" << beta2 << " gamma2=" << gamma2 << std::endl;
    [[maybe_unused]] double const eta2 = beta2 / (1 - beta2);
    HEPMassType const Wmax =
        2 * me * c2 * beta2 * gamma2 / (1 + 2 * gamma * me / m + me2 / m2);
    // approx, but <<1%    HEPMassType const Wmax = 2*me*c2*beta2*gamma2;      for HEAVY
    // PARTICLES Wmax ~ 2me v2 for non-relativistic particles
    std::cout << "BetheBloch Wmax=" << Wmax << std::endl;

    // Sternheimer parameterization, density corrections towards high energies
    // NOTE/TODO: when Cbar is 0 it needs to be approximated from parameterization ->
    // MISSING
    std::cout << "BetheBloch p.GetMomentum().GetNorm()/m="
              << p.GetMomentum().GetNorm() / m << std::endl;
    double const x = log10(p.GetMomentum().GetNorm() / m);
    double delta = 0;
    if (x >= x1) {
      delta = 2 * (log(10)) * x - Cbar;
    } else if (x < x1 && x >= x0) {
      delta = 2 * (log(10)) * x - Cbar + aa * pow((x1 - x), sk);
    } else if (x < x0) { // and IF conductor (otherwise, this is 0)
      delta = delta0 * pow(100, 2 * (x - x0));
    }
    std::cout << "BetheBloch delta=" << delta << std::endl;

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
    // if (p.GetPID() == corsika::Code::Nucleus) A = p.GetNuclearA();
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

    // std::cout << "BetheBloch Erel=" << Erel << " barkas=" << barkas << " bloch=" <<
    // bloch << std::endl;

    double const aux = 2 * me * c2 * beta2 * gamma2 * Wmax / (Ieff * Ieff);
    return -K * Z2 * ZoverA / beta2 *
           (0.5 * log(aux) - beta2 - Cadj / Z - delta / 2 + barkas + bloch) * dX;
  }

  // radiation losses according to PDG 2018, ch. 33 ref. [5]
  HEPEnergyType BetheBlochPDG::RadiationLosses(SetupParticle const& vP,
                                               GrammageType const vDX) {
    // simple-minded hard-coded value for b(E) inspired by data from
    // http://pdg.lbl.gov/2018/AtomicNuclearProperties/ for N and O.
    auto constexpr b = 3.0 * 1e-6 * square(1_cm) / 1_g;
    return -vP.GetEnergy() * b * vDX;
  }

  HEPEnergyType BetheBlochPDG::TotalEnergyLoss(SetupParticle const& vP,
                                               GrammageType const vDX) {
    return BetheBloch(vP, vDX) + RadiationLosses(vP, vDX);
  }

  corsika::EProcessReturn BetheBlochPDG::DoContinuous(SetupParticle& p,
                                                      SetupTrack const& t) {
    if (p.GetChargeNumber() == 0) return corsika::EProcessReturn::eOk;

    GrammageType const dX =
        p.GetNode()->GetModelProperties().IntegratedGrammage(t, t.GetLength());
    std::cout << "BetheBlochPDG " << p.GetPID() << ", z=" << p.GetChargeNumber()
              << ", dX=" << dX / 1_g * square(1_cm) << "g/cm2" << std::endl;
    HEPEnergyType dE = TotalEnergyLoss(p, dX);
    auto E = p.GetEnergy();
    const auto Ekin = E - p.GetMass();
    auto Enew = E + dE;
    std::cout << "BetheBlochPDG  dE=" << dE / 1_MeV << "MeV, "
              << " E=" << E / 1_GeV << "GeV,  Ekin=" << Ekin / 1_GeV
              << ", Enew=" << Enew / 1_GeV << "GeV" << std::endl;
    auto status = corsika::EProcessReturn::eOk;
    if (-dE > Ekin) {
      dE = -Ekin;
      Enew = p.GetMass();
      status = corsika::EProcessReturn::eParticleAbsorbed;
    }
    p.SetEnergy(Enew);
    MomentumUpdate(p, Enew);
    BetheBlochPDGTot_ += dE;
    FillProfile(p, t, dE);
    return status;
  }

  LengthType BetheBlochPDG::MaxStepLength(SetupParticle const& vParticle,
                                          SetupTrack const& vTrack) const {
    if (vParticle.GetChargeNumber() == 0) {
      return meter * std::numeric_limits<double>::infinity();
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

  void BetheBlochPDG::MomentumUpdate(corsika::setup::Stack::ParticleType& vP,
                                     HEPEnergyType Enew) {
    HEPMomentumType Pnew = elab2plab(Enew, vP.GetMass());
    auto pnew = vP.GetMomentum();
    vP.SetMomentum(pnew * Pnew / pnew.GetNorm());
  }

  void BetheBlochPDG::FillProfile(SetupParticle const& vP, SetupTrack const& vTrack,
                                  const HEPEnergyType dE) {

    auto const toStart = vTrack.GetPosition(0) - InjectionPoint_;
    auto const toEnd = vTrack.GetPosition(1) - InjectionPoint_;

    auto const v1 = (toStart * 1_Hz).dot(ShowerAxisDirection_);
    auto const v2 = (toEnd * 1_Hz).dot(ShowerAxisDirection_);
    corsika::Line const lineToStartBin(InjectionPoint_, ShowerAxisDirection_ * v1);
    corsika::Line const lineToEndBin(InjectionPoint_, ShowerAxisDirection_ * v2);

    SetupTrack const trajToStartBin(lineToStartBin, 1_s);
    SetupTrack const trajToEndBin(lineToEndBin, 1_s);

    GrammageType const grammageStart =
        vP.GetNode()->GetModelProperties().IntegratedGrammage(trajToStartBin,
                                                              trajToStartBin.GetLength());
    GrammageType const grammageEnd =
        vP.GetNode()->GetModelProperties().IntegratedGrammage(trajToEndBin,
                                                              trajToEndBin.GetLength());

    const int binStart = grammageStart / dX_;
    const int binEnd = grammageEnd / dX_;

    std::cout << "energy deposit of " << -dE << " between " << grammageStart << " and "
              << grammageEnd << std::endl;

    auto energyCount = HEPEnergyType::zero();

    auto fill = [&](int bin, GrammageType weight) {
      const auto dX = grammageEnd - grammageStart;
      if (dX > dX_threshold_) {
        auto const increment = -dE * weight / (grammageEnd - grammageStart);
        Profile_[bin] += increment;
        energyCount += increment;

        std::cout << "filling bin " << bin << " with weight " << weight << ": "
                  << increment << std::endl;
      }
    };

    // fill longitudinal profile
    fill(binStart, (1 + binStart) * dX_ - grammageStart);
    fill(binEnd, grammageEnd - binEnd * dX_);

    if (binStart == binEnd) { fill(binStart, -dX_); }

    for (int bin = binStart + 1; bin < binEnd; ++bin) { fill(bin, dX_); }

    std::cout << "total energy added to histogram: " << energyCount << std::endl;
  }

  void BetheBlochPDG::PrintProfile() const {
    std::ofstream file("BetheBlochPDGProfile.dat");
    std::cout << "# BetheBlochPDG PrintProfile  X-bin [g/cm2]  dE/dX [GeV/g/cm2]  "
              << std::endl;
    double const deltaX = dX_ / 1_g * square(1_cm);
    for (auto v : Profile_) {
      file << v.first * deltaX << " " << v.second / (deltaX * 1_GeV) << std::endl;
    }
  }

} // namespace corsika::energy_loss
