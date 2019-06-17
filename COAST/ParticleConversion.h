/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_processes_sibyll_particles_h_
#define _include_processes_sibyll_particles_h_

#include <corsika/particles/ParticleProperties.h>

#include <map>

namespace corsika::coast {

  /**
     Here we define the original CORSIKA particle codes, see corsika manual
   */
  enum class CoastCode : int16_t {
    Gamma = 1,
    Positron = 2,
    Electron = 3,
    MuonBar = 5,
    Muon = 6,
    Pi0 = 7,
    PiP = 8,
    PiM = 9,
    Klong = 10,
    KP = 11,
    KM = 12,
    Neutron = 13,
    Proton = 14,
    ProtonBar = 15,
    Kshort = 16,
    Eta = 17,
    Lambda = 18,
    SigmaPlus = 19,
    Sigma0 = 20,
    SigmaMinus = 21,
    Xi0 = 22,
    XiMinus = 23,
    OmegaMinus = 24,
    NeutronBar = 25,
    LambdaBar = 26,
    SigmaMinusBar = 27,
    Sigma0Bar = 28,
    SigmaPlusBar = 29,
    Xi0Bar = 30,
    XiPlusBar = 31,
    OmegaPlusBar = 32,

    EtaPrime = 48,
    Phi = 49,
    omega = 50,
    Rho0 = 51,
    RhoPlus = 52,
    RhoMinus = 53,
    DeltaPlusPlus = 54,
    DeltaPlus = 55,
    Delta0 = 56,
    DeltaMinus = 57,
    DeltaMinusMinusBar = 58,
    DeltaMinusBar = 59,
    Delta0Bar = 60,
    DeltaPlusBar = 61,
    KStar0 = 62,
    KStarPlus = 63,
    KStarMinus = 64,
    KStar0Bar = 65,

    NeutrinoE = 66,
    NeutrinoEBar = 67,
    NeutrinoMu = 68,
    NeutrinoMuBar = 69,

    Code71 = 71,
    Code72 = 72,
    Code73 = 73,
    Code74 = 74,
    Code75 = 75,
    Code76 = 76,

    Code85 = 85,
    Code86 = 86,

    Code95 = 95,
    Code96 = 96,

    ProtonNucleus = 101,
    Deuterium = 201,
    Tritium = 301,
    He3 = 302,
    Helium = 402,
    Lithium = 603,
    Beryllium = 904,
    Boron = 1005,
    Carbon = 1206,
    Carbon13 = 1306,
    Nitrogen = 1407,
    Oxygen = 1608,
    Fluor = 1809,
    Neon21 = 2110,
    Neon = 2210,
    Argon = 1838,
    Iron = 5628,
    Xenon = 12854,
    Radon = 13888,
  };

  using CoastCodeIntType = std::underlying_type<CoastCode>::type;

  /**
     Here we convert CORSIKA7 to CORSIKA8 codes
   */

  const std::map<corsika::coast::CoastCode, corsika::particles::Code> coast2corsika = {
      {CoastCode::Gamma, corsika::particles::Code::Gamma},
      {CoastCode::Positron, corsika::particles::Code::Positron},
      {CoastCode::Electron, corsika::particles::Code::Electron},
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // 4
      {CoastCode::MuonBar, corsika::particles::Code::MuPlus},
      {CoastCode::Muon, corsika::particles::Code::MuMinus},
      {CoastCode::Pi0, corsika::particles::Code::Pi0},
      {CoastCode::PiP, corsika::particles::Code::PiPlus},
      {CoastCode::PiM, corsika::particles::Code::PiMinus},
      {CoastCode::Klong, corsika::particles::Code::K0Long}, // 10
      {CoastCode::KP, corsika::particles::Code::KPlus},
      {CoastCode::KM, corsika::particles::Code::KMinus},
      {CoastCode::Neutron, corsika::particles::Code::Neutron},
      {CoastCode::Proton, corsika::particles::Code::Proton}, // 14
      {CoastCode::ProtonBar, corsika::particles::Code::AntiProton},
      {CoastCode::Kshort, corsika::particles::Code::K0Short},
      {CoastCode::Eta, corsika::particles::Code::Eta}, // 17
      {CoastCode::Lambda, corsika::particles::Code::Lambda0},
      {CoastCode::SigmaPlus, corsika::particles::Code::SigmaPlus},
      {CoastCode::Sigma0, corsika::particles::Code::Sigma0}, // 20
      {CoastCode::SigmaMinus, corsika::particles::Code::SigmaMinus},
      {CoastCode::Xi0, corsika::particles::Code::Xi0},
      {CoastCode::XiMinus, corsika::particles::Code::XiMinus},
      {CoastCode::OmegaMinus, corsika::particles::Code::OmegaMinus},
      {CoastCode::NeutronBar, corsika::particles::Code::AntiNeutron}, // 25
      {CoastCode::LambdaBar, corsika::particles::Code::Lambda0Bar},
      {CoastCode::SigmaMinusBar, corsika::particles::Code::SigmaMinusBar},
      {CoastCode::Sigma0Bar, corsika::particles::Code::Sigma0Bar},
      {CoastCode::SigmaPlusBar, corsika::particles::Code::SigmaPlusBar},
      {CoastCode::Xi0Bar, corsika::particles::Code::Xi0Bar},
      {CoastCode::XiPlusBar, corsika::particles::Code::XiPlusBar},
      {CoastCode::OmegaPlusBar, corsika::particles::Code::OmegaPlusBar}, // 32
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // eta-prime
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // PHI
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // omega
      {CoastCode::Rho0, corsika::particles::Code::Rho0}, // 51
      {CoastCode::RhoPlus, corsika::particles::Code::RhoPlus},
      {CoastCode::RhoMinus, corsika::particles::Code::RhoMinus},
      {CoastCode::DeltaPlusPlus, corsika::particles::Code::DeltaPlusPlus},
      {CoastCode::DeltaPlus, corsika::particles::Code::DeltaPlus},
      {CoastCode::Delta0, corsika::particles::Code::Delta0}, // 56
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // DeltaMinus},
      {CoastCode::DeltaMinusMinusBar, corsika::particles::Code::DeltaMinusMinusBar},
      {CoastCode::DeltaMinusBar, corsika::particles::Code::DeltaMinusBar},
      {CoastCode::Delta0Bar, corsika::particles::Code::Delta0Bar},
      //{CoastCode::    ,corsika::particles::Code::Unknown}, // DeltaPlusBar
      {CoastCode::KStar0, corsika::particles::Code::KStar0}, // 62
      {CoastCode::KStarPlus, corsika::particles::Code::KStarPlus},
      {CoastCode::KStarMinus, corsika::particles::Code::KStarMinus},
      {CoastCode::KStar0Bar, corsika::particles::Code::KStar0Bar},
      {CoastCode::NeutrinoE, corsika::particles::Code::NuE},
      {CoastCode::NeutrinoEBar, corsika::particles::Code::NuEBar},
      {CoastCode::NeutrinoMu, corsika::particles::Code::NuMu},
      {CoastCode::NeutrinoMuBar, corsika::particles::Code::NuMuBar}, // 69
      {CoastCode::Code71, corsika::particles::Code::Unknown},
      {CoastCode::Code72, corsika::particles::Code::Unknown},
      {CoastCode::Code73, corsika::particles::Code::Unknown},
      {CoastCode::Code74, corsika::particles::Code::Unknown},
      {CoastCode::Code75, corsika::particles::Code::Unknown},
      {CoastCode::Code76, corsika::particles::Code::Unknown},
      {CoastCode::Code85, corsika::particles::Code::Unknown},
      {CoastCode::Code86, corsika::particles::Code::Unknown},
      {CoastCode::Code95, corsika::particles::Code::Unknown},
      {CoastCode::Code96, corsika::particles::Code::Unknown},

      {CoastCode::ProtonNucleus, corsika::particles::Code::Proton}, // 101
      {CoastCode::Deuterium, corsika::particles::Code::Deuterium},
      {CoastCode::Tritium, corsika::particles::Code::Tritium},
      {CoastCode::He3, corsika::particles::Code::Helium3},
      {CoastCode::Helium, corsika::particles::Code::Helium}, // 402
      //{CoastCode::Lithium, corsika::particles::Code::Lithium},
      //{CoastCode::Beryllium, corsika::particles::Code::Beryllium},
      //{CoastCode::Boron, corsika::particles::Code::Boron},
      //{CoastCode::Carbon, corsika::particles::Code::Carbon},
      //{CoastCode::Carbon13, corsika::particles::Code::Carbon13},
      //{CoastCode::Nitrogen, corsika::particles::Code::Nitrogen},
      //{CoastCode::Fluor, corsika::particles::Code::Fluor},
      //{CoastCode::Oxygen, corsika::particles::Code::Oxygen},
      //{CoastCode::Neon21, corsika::particles::Code::Neon21},
      //{CoastCode::Neon, corsika::particles::Code::Neon},
      //{CoastCode::Argon, corsika::particles::Code::Argon},
      //{CoastCode::Xenon, corsika::particles::Code::Xenon},
      //{CoastCode::Radon, corsika::particles::Code::Radon},
      {CoastCode::Iron, corsika::particles::Code::Iron}, // 5628

      //{CoastCode::, corsika::particles::Code::},

  };

  corsika::particles::Code ConvertFromCoast(CoastCode pCode);

} // namespace corsika::coast

#endif
