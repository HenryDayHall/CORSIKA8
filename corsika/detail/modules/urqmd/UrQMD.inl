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

#include <corsika/modules/urqmd/UrQMD.hpp>

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <algorithm>
#include <functional>
#include <iostream>

#include <urqmd.hpp>

namespace corsika::urqmd {

  UrQMD::UrQMD() { ::urqmd::iniurqmdc8_(); }

  using SetupStack = corsika::setup::Stack;
  using SetupParticle = corsika::setup::Stack::StackIterator;
  using SetupProjectile = corsika::setup::StackView::StackIterator;

  CrossSectionType UrQMD::GetCrossSection(corsika::Code vProjectileCode,
                                          corsika::Code vTargetCode,
                                          HEPEnergyType vLabEnergy,
                                          int vAProjectile = 1) {

    // the following is a translation of ptsigtot() into C++
    if (vProjectileCode != corsika::Code::Nucleus &&
        !corsika::is_nucleus(vTargetCode)) { // both particles are "special"
      auto const mProj = corsika::get_mass(vProjectileCode);
      auto const mTar = corsika::get_mass(vTargetCode);
      double sqrtS =
          sqrt(static_pow<2>(mProj) + static_pow<2>(mTar) + 2 * vLabEnergy * mTar) *
          (1 / 1_GeV);

      // we must set some UrQMD globals first...
      auto const [ityp, iso3] = ConvertToUrQMD(vProjectileCode);
      ::urqmd::inputs_.spityp[0] = ityp;
      ::urqmd::inputs_.spiso3[0] = iso3;

      auto const [itypTar, iso3Tar] = ConvertToUrQMD(vTargetCode);
      ::urqmd::inputs_.spityp[1] = itypTar;
      ::urqmd::inputs_.spiso3[1] = iso3Tar;

      int one = 1;
      int two = 2;
      return ::urqmd::sigtot_(one, two, sqrtS) * 1_mb;
    } else {
      int const Ap = vAProjectile;
      int const At = is_nucleus(vTargetCode) ? corsika::get_nucleus_A(vTargetCode) : 1;

      double const maxImpact = ::urqmd::nucrad_(Ap) + ::urqmd::nucrad_(At) +
                               2 * ::urqmd::options_.CTParam[30 - 1];
      return 10_mb * M_PI * static_pow<2>(maxImpact);
      // is a constant cross-section really reasonable?
    }
  }

  template <typename TParticle> // need template here, as this is called both with
                                // SetupParticle as well as SetupProjectile
  CrossSectionType UrQMD::GetCrossSection(TParticle const& vProjectile,
                                          corsika::Code vTargetCode) const {
    // TODO: return 0 for non-hadrons?

    auto const projectileCode = vProjectile.GetPID();
    auto const projectileEnergyLab = vProjectile.GetEnergy();

    if (projectileCode == corsika::Code::K0Long) {
      return 0.5 *
             (GetCrossSection(corsika::Code::K0, vTargetCode, projectileEnergyLab) +
              GetCrossSection(corsika::Code::K0Bar, vTargetCode, projectileEnergyLab));
    }

    int const Ap =
        (projectileCode == corsika::Code::Nucleus) ? vProjectile.GetNuclearA() : 1;
    return GetCrossSection(projectileCode, vTargetCode, projectileEnergyLab, Ap);
  }

  bool UrQMD::CanInteract(corsika::Code vCode) const {
    // According to the manual, UrQMD can use all mesons, baryons and nucleons
    // which are modeled also as input particles. I think it is safer to accept
    // only the usual long-lived species as input.
    // TODO: Charmed mesons should be added to the list, too

    static corsika::Code const validProjectileCodes[] = {
        corsika::Code::Nucleus, corsika::Code::Proton,      corsika::Code::AntiProton,
        corsika::Code::Neutron, corsika::Code::AntiNeutron, corsika::Code::PiPlus,
        corsika::Code::PiMinus, corsika::Code::KPlus,       corsika::Code::KMinus,
        corsika::Code::K0,      corsika::Code::K0Bar,       corsika::Code::K0Long};

    return std::find(std::cbegin(validProjectileCodes), std::cend(validProjectileCodes),
                     vCode) != std::cend(validProjectileCodes);
  }

  GrammageType UrQMD::GetInteractionLength(SetupParticle& vParticle) const {

    if (!CanInteract(vParticle.GetPID())) {
      // we could do the canInteract check in GetCrossSection, too but if
      // we do it here we have the advantage of avoiding the loop
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }

    auto const& mediumComposition =
        vParticle.GetNode()->GetModelProperties().getNuclearComposition();
    using namespace std::placeholders;

    CrossSectionType const weightedProdCrossSection = mediumComposition.WeightedSum(
        std::bind(&UrQMD::GetCrossSection<decltype(vParticle)>, this, vParticle, _1));

    return mediumComposition.GetAverageMassNumber() * constants::u /
           weightedProdCrossSection;
  }

  void UrQMD::doInteraction(SetupProjectile& vProjectile) {

    auto projectileCode = vProjectile.GetPID();
    auto const projectileEnergyLab = vProjectile.GetEnergy();
    auto const& projectileMomentumLab = vProjectile.GetMomentum();
    auto const& projectilePosition = vProjectile.GetPosition();
    auto const projectileTime = vProjectile.GetTime();

    // sample target particle
    auto const& mediumComposition =
        vProjectile.GetNode()->GetModelProperties().getNuclearComposition();
    auto const componentCrossSections = std::invoke([&]() {
      auto const& components = mediumComposition.GetComponents();
      std::vector<CrossSectionType> crossSections;
      crossSections.reserve(components.size());

      for (auto const c : components) {
        crossSections.push_back(GetCrossSection(vProjectile, c));
      }

      return crossSections;
    });

    auto const targetCode = mediumComposition.SampleTarget(componentCrossSections, fRNG);
    auto const targetA = corsika::get_nucleus_A(targetCode);
    auto const targetZ = corsika::get_nucleus_Z(targetCode);

    ::urqmd::inputs_.nevents = 1;
    ::urqmd::sys_.eos = 0; // could be configurable in principle
    ::urqmd::inputs_.outsteps = 1;
    ::urqmd::sys_.nsteps = 1;

    // initialization regarding projectile
    if (corsika::Code::Nucleus == projectileCode) {
      // is this everything?
      ::urqmd::inputs_.prspflg = 0;

      ::urqmd::sys_.Ap = vProjectile.GetNuclearA();
      ::urqmd::sys_.Zp = vProjectile.GetNuclearZ();
      ::urqmd::rsys_.ebeam = (projectileEnergyLab - vProjectile.GetMass()) * (1 / 1_GeV) /
                             vProjectile.GetNuclearA();

      ::urqmd::rsys_.bdist = ::urqmd::nucrad_(targetA) +
                             ::urqmd::nucrad_(::urqmd::sys_.Ap) +
                             2 * ::urqmd::options_.CTParam[30 - 1];

      int const id = 1;
      ::urqmd::cascinit_(::urqmd::sys_.Zp, ::urqmd::sys_.Ap, id);
    } else {
      ::urqmd::inputs_.prspflg = 1;
      ::urqmd::sys_.Ap =
          1; // even for non-baryons this has to be set, see vanilla UrQMD.f
      ::urqmd::rsys_.bdist = ::urqmd::nucrad_(targetA) + ::urqmd::nucrad_(1) +
                             2 * ::urqmd::options_.CTParam[30 - 1];
      ::urqmd::rsys_.ebeam = (projectileEnergyLab - vProjectile.GetMass()) * (1 / 1_GeV);

      if (projectileCode == corsika::Code::K0Long) {
        projectileCode = fBooleanDist(fRNG) ? corsika::Code::K0 : corsika::Code::K0Bar;
      } else if (projectileCode == corsika::Code::K0Short) {
        throw std::runtime_error("K0Short should not interact");
      }

      auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
      // todo: conversion of K_long/short into strong eigenstates;
      ::urqmd::inputs_.spityp[0] = ityp;
      ::urqmd::inputs_.spiso3[0] = iso3;
    }

    // initilazation regarding target
    if (corsika::is_nucleus(targetCode)) {
      ::urqmd::sys_.Zt = targetZ;
      ::urqmd::sys_.At = targetA;
      ::urqmd::inputs_.trspflg = 0; // nucleus as target
      int const id = 2;
      ::urqmd::cascinit_(::urqmd::sys_.Zt, ::urqmd::sys_.At, id);
    } else {
      ::urqmd::inputs_.trspflg = 1; // special particle as target
      auto const [ityp, iso3] = ConvertToUrQMD(targetCode);
      ::urqmd::inputs_.spityp[1] = ityp;
      ::urqmd::inputs_.spiso3[1] = iso3;
    }

    int iflb = 0; // flag for retrying interaction in case of empty event, 0 means retry
    ::urqmd::urqmd_(iflb);

    // now retrieve secondaries from UrQMD
    auto const& originalCS = projectileMomentumLab.GetCoordinateSystem();
    corsika::CoordinateSystem const zAxisFrame =
        originalCS.RotateToZ(projectileMomentumLab);

    for (int i = 0; i < ::urqmd::sys_.npart; ++i) {
      auto code = ConvertFromUrQMD(::urqmd::isys_.ityp[i], ::urqmd::isys_.iso3[i]);
      if (code == corsika::Code::K0 || code == corsika::Code::K0Bar) {
        code = fBooleanDist(fRNG) ? corsika::Code::K0Short : corsika::Code::K0Long;
      }

      // "coor_.p0[i] * 1_GeV" is likely off-shell as UrQMD doesn't preserve masses well
      auto momentum = corsika::Vector(
          zAxisFrame,
          corsika::QuantityVector<dimensionless_d>{
              ::urqmd::coor_.px[i], ::urqmd::coor_.py[i], ::urqmd::coor_.pz[i]} *
              1_GeV);

      auto const energy = sqrt(momentum.squaredNorm() + square(corsika::get_mass(code)));

      momentum.rebase(originalCS); // transform back into standard lab frame
      std::cout << i << " " << code << " " << momentum.GetComponents() << std::endl;

      vProjectile.AddSecondary(
          std::tuple<corsika::Code, HEPEnergyType, corsika::MomentumVector,
                     corsika::Point, TimeType>{code, energy, momentum, projectilePosition,
                                               projectileTime});
    }

    std::cout << "UrQMD generated " << ::urqmd::sys_.npart << " secondaries!"
              << std::endl;
  }

  corsika::Code ConvertFromUrQMD(int vItyp, int vIso3) {
    int const pdgInt =
        ::urqmd::pdgid_(vItyp, vIso3); // use the conversion function provided by UrQMD
    if (pdgInt == 0) {                 // ::urqmd::pdgid_ returns 0 on error
      throw std::runtime_error("UrQMD pdgid() returned 0");
    }
    auto const pdg = static_cast<corsika::PDGCode>(pdgInt);
    return corsika::convert_from_PDG(pdg);
  }

  std::pair<int, int> ConvertToUrQMD(corsika::Code code) {
    static const std::map<int, std::pair<int, int>> mapPDGToUrQMD{
        // data mostly from github.com/afedynitch/ParticleDataTool
        {22, {100, 0}},      // gamma
        {111, {101, 0}},     // pi0
        {211, {101, 2}},     // pi+
        {-211, {101, -2}},   // pi-
        {321, {106, 1}},     // K+
        {-321, {-106, -1}},  // K-
        {311, {106, -1}},    // K0
        {-311, {-106, 1}},   // K0bar
        {2212, {1, 1}},      // p
        {2112, {1, -1}},     // n
        {-2212, {-1, -1}},   // pbar
        {-2112, {-1, 1}},    // nbar
        {221, {102, 0}},     // eta
        {213, {104, 2}},     // rho+
        {-213, {104, -2}},   // rho-
        {113, {104, 0}},     // rho0
        {323, {108, 2}},     // K*+
        {-323, {108, -2}},   // K*-
        {313, {108, 0}},     // K*0
        {-313, {-108, 0}},   // K*0-bar
        {223, {103, 0}},     // omega
        {333, {109, 0}},     // phi
        {3222, {40, 2}},     // Sigma+
        {3212, {40, 0}},     // Sigma0
        {3112, {40, -2}},    // Sigma-
        {3322, {49, 0}},     // Xi0
        {3312, {49, -1}},    // Xi-
        {3122, {27, 0}},     // Lambda0
        {2224, {17, 4}},     // Delta++
        {2214, {17, 2}},     // Delta+
        {2114, {17, 0}},     // Delta0
        {1114, {17, -2}},    // Delta-
        {3224, {41, 2}},     // Sigma*+
        {3214, {41, 0}},     // Sigma*0
        {3114, {41, -2}},    // Sigma*-
        {3324, {50, 0}},     // Xi*0
        {3314, {50, -1}},    // Xi*-
        {3334, {55, 0}},     // Omega-
        {411, {133, 2}},     // D+
        {-411, {133, -2}},   // D-
        {421, {133, 0}},     // D0
        {-421, {-133, 0}},   // D0-bar
        {441, {107, 0}},     // etaC
        {431, {138, 1}},     // Ds+
        {-431, {138, -1}},   // Ds-
        {433, {139, 1}},     // Ds*+
        {-433, {139, -1}},   // Ds*-
        {413, {134, 1}},     // D*+
        {-413, {134, -1}},   // D*-
        {10421, {134, 0}},   // D*0
        {-10421, {-134, 0}}, // D*0-bar
        {443, {135, 0}},     // jpsi
    };

    return mapPDGToUrQMD.at(static_cast<int>(get_PDG(code)));
  }

} // namespace corsika::urqmd
