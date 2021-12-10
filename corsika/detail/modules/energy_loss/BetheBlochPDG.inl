/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/ParticleProperties.hpp>

#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/core/Logging.hpp>

#include <cmath>
#include <fstream>
#include <limits>

namespace corsika {

  auto elab2plab = [](HEPEnergyType Elab, HEPMassType m) {
    return sqrt((Elab - m) * (Elab + m));
  };

  inline BetheBlochPDG::BetheBlochPDG(ShowerAxis const& shower_axis)
      : dX_(10_g / square(1_cm)) // profile binning
      , dX_threshold_(0.0001_g / square(1_cm))
      , shower_axis_(shower_axis)
      , profile_(int(shower_axis.getMaximumX() / dX_) + 1) {}

  template <typename TParticle>
  inline HEPEnergyType BetheBlochPDG::getBetheBloch(TParticle const& p,
                                                    GrammageType const dX) {

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
    HEPEnergyType const E = p.getEnergy();
    HEPMassType const m = p.getMass();
    double const gamma = E / m;
    int const Z = p.getChargeNumber();
    int const Z2 = Z * Z;
    HEPMassType constexpr me = Electron::mass;
    auto const m2 = m * m;
    auto constexpr me2 = me * me;
    double const gamma2 = gamma * gamma;

    double const beta2 = (gamma2 - 1) / gamma2; // 1-1/gamma2    (1-1/gamma)*(1+1/gamma);
                                                // (gamma_2-1)/gamma_2 = (1-1/gamma2);
    double constexpr c2 = 1;                    // HEP convention here c=c2=1
    CORSIKA_LOG_TRACE("BetheBloch beta2={}, gamma2={}", beta2, gamma2);
    [[maybe_unused]] double const eta2 = beta2 / (1 - beta2);
    HEPMassType const Wmax =
        2 * me * c2 * beta2 * gamma2 / (1 + 2 * gamma * me / m + me2 / m2);
    // approx, but <<1%    HEPMassType const Wmax = 2*me*c2*beta2*gamma2;      for HEAVY
    // PARTICLES Wmax ~ 2me v2 for non-relativistic particles
    CORSIKA_LOG_TRACE("BetheBloch Wmax={}", Wmax);

    // Sternheimer parameterization, density corrections towards high energies
    // NOTE/TODO: when Cbar is 0 it needs to be approximated from parameterization ->
    // MISSING
    CORSIKA_LOG_TRACE("BetheBloch p.getMomentum().getNorm()/m={}",
                      p.getMomentum().getNorm() / m);
    double const x = log10(p.getMomentum().getNorm() / m);
    double delta = 0;
    if (x >= x1) {
      delta = 2 * (log(10)) * x - Cbar;
    } else if (x < x1 && x >= x0) {
      delta = 2 * (log(10)) * x - Cbar + aa * pow((x1 - x), sk);
    } else if (x < x0) { // and IF conductor (otherwise, this is 0)
      delta = delta0 * pow(100, 2 * (x - x0));
    }
    CORSIKA_LOG_TRACE("BetheBloch delta={}", delta);

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
    // if (p.getPID() == Code::Nucleus) A = p.getNuclearA();
    // double const Erel = (p.getEnergy()-p.getMass()) / A / 1_keV;
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

    double const aux = 2 * me * c2 * beta2 * gamma2 * Wmax / (Ieff * Ieff);
    return -K * Z2 * ZoverA / beta2 *
           (0.5 * log(aux) - beta2 - Cadj / Z - delta / 2 + barkas + bloch) * dX;
  }

  // radiation losses according to PDG 2018, ch. 33 ref. [5]
  template <typename TParticle>
  inline HEPEnergyType BetheBlochPDG::getRadiationLosses(TParticle const& vP,
                                                         GrammageType const vDX) {
    // simple-minded hard-coded value for b(E) inspired by data from
    // http://pdg.lbl.gov/2018/AtomicNuclearProperties/ for N and O.
    auto constexpr b = 3.0 * 1e-6 * square(1_cm) / 1_g;
    return -vP.getEnergy() * b * vDX;
  }

  template <typename TParticle>
  inline HEPEnergyType BetheBlochPDG::getTotalEnergyLoss(TParticle const& vP,
                                                         GrammageType const vDX) {
    return getBetheBloch(vP, vDX) + getRadiationLosses(vP, vDX);
  }

  template <typename TParticle, typename TTrajectory>
  inline ProcessReturn BetheBlochPDG::doContinuous(TParticle& p, TTrajectory const& t,
                                                   bool const) {

    // if this step was limiting the CORSIKA stepping, the particle is lost
    /* see Issue https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/-/issues/389
    if (limitStep) {
      fillProfile(t, p.getEnergy());
      p.setEnergy(p.getMass());
      return ProcessReturn::ParticleAbsorbed;
    }
    */

    if (p.getChargeNumber() == 0) return ProcessReturn::Ok;

    GrammageType const dX = p.getNode()->getModelProperties().getIntegratedGrammage(t);
    CORSIKA_LOG_TRACE("EnergyLoss pid={}, z={}, dX={} g/cm2", p.getPID(),
                      p.getChargeNumber(), dX / 1_g * square(1_cm));
    HEPEnergyType dE = getTotalEnergyLoss(p, dX);
    auto E = p.getEnergy();
    [[maybe_unused]] const auto Ekin = E - p.getMass();
    auto Enew = E + dE;
    CORSIKA_LOG_TRACE("EnergyLoss  dE={} MeV, E={} GeV, Ekin={} GeV, Enew={} GeV",
                      dE / 1_MeV, E / 1_GeV, Ekin / 1_GeV, Enew / 1_GeV);
    p.setEnergy(Enew); // kinetic energy on stack
    fillProfile(t, dE);
    return ProcessReturn::Ok;
  }

  template <typename TParticle, typename TTrajectory>
  inline LengthType BetheBlochPDG::getMaxStepLength(TParticle const& vParticle,
                                                    TTrajectory const& vTrack) const {
    if (vParticle.getChargeNumber() == 0) {
      return meter * std::numeric_limits<double>::infinity();
    }

    auto constexpr dX = 1_g / square(1_cm);
    auto const dEdX = -getTotalEnergyLoss(vParticle, dX) / dX;
    auto const energy = vParticle.getKineticEnergy();
    auto const energy_lim =
        std::max(energy * 0.9, // either 10% relative loss max., or
                 calculate_kinetic_energy_threshold(
                     vParticle.getPID()) // energy thresholds globally defined
                                         // for individual particles
                     * 0.99999 // need to go slightly below global e-cut to assure
                               // removal in ParticleCut. The 1% does not matter since
                               // at cut-time the entire energy is removed.
        );
    auto const maxGrammage = (energy - energy_lim) / dEdX;

    return vParticle.getNode()->getModelProperties().getArclengthFromGrammage(
        vTrack, maxGrammage);
  }

  template <typename TTrajectory>
  inline void BetheBlochPDG::fillProfile(TTrajectory const& vTrack,
                                         const HEPEnergyType dE) {

    GrammageType grammageStart = shower_axis_.getProjectedX(vTrack.getPosition(0));
    GrammageType grammageEnd = shower_axis_.getProjectedX(vTrack.getPosition(1));

    if (grammageStart > grammageEnd) { // particle going upstream
      std::swap(grammageStart, grammageEnd);
    }

    GrammageType const deltaX = grammageEnd - grammageStart;

    if (deltaX < dX_threshold_) return;

    // only register the range that is covered by the profile
    int const maxBin = int(profile_.size() - 1);
    int binStart = grammageStart / dX_;
    if (binStart < 0) binStart = 0;
    if (binStart > maxBin) binStart = maxBin;
    int binEnd = grammageEnd / dX_;
    if (binEnd < 0) binEnd = 0;
    if (binEnd > maxBin) binEnd = maxBin;

    CORSIKA_LOG_TRACE("energy deposit of -dE={} between {} and {}", -dE, grammageStart,
                      grammageEnd);

    auto energyCount = HEPEnergyType::zero();

    auto const factor = -dE / deltaX;
    auto fill = [&](int const bin, GrammageType const weight) {
      auto const increment = factor * weight;
      profile_[bin] += increment;
      energyCount += increment;

      CORSIKA_LOG_TRACE("filling bin {} with weight {} : {} ", bin, weight, increment);
    };

    // fill longitudinal profile
    if (binStart == binEnd) {
      fill(binStart, deltaX);
    } else {
      fill(binStart, ((1 + binStart) * dX_ - grammageStart));
      fill(binEnd, (grammageEnd - binEnd * dX_));
      for (int bin = binStart + 1; bin < binEnd; ++bin) { fill(bin, dX_); }
    }

    CORSIKA_LOG_TRACE("total energy added to histogram: {} ", energyCount);
  }

  inline void BetheBlochPDG::printProfile() const {
    std::ofstream file("EnergyLossProfile.dat");
    file << "# EnergyLoss profile" << std::endl
         << "# lower X bin edge [g/cm2]  dE/dX [GeV/g/cm2]\n";
    double const deltaX = dX_ / 1_g * square(1_cm);
    for (size_t i = 0; i < profile_.size(); ++i) {
      file << std::scientific << std::setw(15) << i * deltaX << std::setw(15)
           << profile_.at(i) / (deltaX * 1_GeV) << '\n';
    }
    file.close();
  }

  inline HEPEnergyType BetheBlochPDG::getTotal() const {
    return std::accumulate(profile_.cbegin(), profile_.cend(), HEPEnergyType::zero());
  }

  inline void BetheBlochPDG::showResults() const {
    CORSIKA_LOG_INFO(
        " ******************************\n"
        " PROCESS::ContinuousProcess: \n"
        " energy lost dE (GeV)      : {}\n  ",
        energy_lost_ / 1_GeV);
  }

  inline void BetheBlochPDG::reset() { energy_lost_ = 0_GeV; }

} // namespace corsika
