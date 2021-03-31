/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
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

  inline UrQMD::UrQMD() { ::urqmd::iniurqmdc8_(); }

  inline CrossSectionType UrQMD::getCrossSection(Code vProjectileCode, Code vTargetCode,
                                                 HEPEnergyType vLabEnergy,
                                                 int vAProjectile = 1) {

    // the following is a translation of ptsigtot() into C++
    if (vProjectileCode != Code::Nucleus &&
        !is_nucleus(vTargetCode)) { // both particles are "special"
      auto const mProj = get_mass(vProjectileCode);
      auto const mTar = get_mass(vTargetCode);
      double sqrtS =
          sqrt(static_pow<2>(mProj) + static_pow<2>(mTar) + 2 * vLabEnergy * mTar) *
          (1 / 1_GeV);

      // we must set some UrQMD globals first...
      auto const [ityp, iso3] = convertToUrQMD(vProjectileCode);
      ::urqmd::inputs_.spityp[0] = ityp;
      ::urqmd::inputs_.spiso3[0] = iso3;

      auto const [itypTar, iso3Tar] = convertToUrQMD(vTargetCode);
      ::urqmd::inputs_.spityp[1] = itypTar;
      ::urqmd::inputs_.spiso3[1] = iso3Tar;

      int one = 1;
      int two = 2;
      return ::urqmd::sigtot_(one, two, sqrtS) * 1_mb;
    } else {
      int const Ap = vAProjectile;
      int const At = is_nucleus(vTargetCode) ? get_nucleus_A(vTargetCode) : 1;

      double const maxImpact = ::urqmd::nucrad_(Ap) + ::urqmd::nucrad_(At) +
                               2 * ::urqmd::options_.CTParam[30 - 1];
      return 10_mb * M_PI * static_pow<2>(maxImpact);
      // is a constant cross-section really reasonable?
    }
  }

  template <typename TParticle> // need template here, as this is called both with
                                // SetupParticle as well as SetupProjectile
  inline CrossSectionType UrQMD::getCrossSection(TParticle const& vProjectile,
                                                 Code vTargetCode) const {
    // TODO: return 0 for non-hadrons?

    auto const projectileCode = vProjectile.getPID();
    auto const projectileEnergyLab = vProjectile.getEnergy();

    if (projectileCode == Code::K0Long) {
      return 0.5 * (getCrossSection(Code::K0, vTargetCode, projectileEnergyLab) +
                    getCrossSection(Code::K0Bar, vTargetCode, projectileEnergyLab));
    }

    int const Ap = (projectileCode == Code::Nucleus) ? vProjectile.getNuclearA() : 1;
    return getCrossSection(projectileCode, vTargetCode, projectileEnergyLab, Ap);
  }

  inline bool UrQMD::canInteract(Code vCode) const {
    // According to the manual, UrQMD can use all mesons, baryons and nucleons
    // which are modeled also as input particles. I think it is safer to accept
    // only the usual long-lived species as input.
    // TODO: Charmed mesons should be added to the list, too

    static Code const validProjectileCodes[] = {
        Code::Nucleus,     Code::Proton, Code::AntiProton, Code::Neutron,
        Code::AntiNeutron, Code::PiPlus, Code::PiMinus,    Code::KPlus,
        Code::KMinus,      Code::K0,     Code::K0Bar,      Code::K0Long};

    return std::find(std::cbegin(validProjectileCodes), std::cend(validProjectileCodes),
                     vCode) != std::cend(validProjectileCodes);
  }

  template <typename TParticle>
  inline GrammageType UrQMD::getInteractionLength(TParticle const& vParticle) const {

    if (!canInteract(vParticle.getPID())) {
      // we could do the canInteract check in getCrossSection, too but if
      // we do it here we have the advantage of avoiding the loop
      return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
    }

    auto const& mediumComposition =
        vParticle.getNode()->getModelProperties().getNuclearComposition();
    using namespace std::placeholders;

    CrossSectionType const weightedProdCrossSection = mediumComposition.getWeightedSum(
        std::bind(&UrQMD::getCrossSection<decltype(vParticle)>, this, vParticle, _1));

    return mediumComposition.getAverageMassNumber() * constants::u /
           weightedProdCrossSection;
  }

  template <typename TView>
  inline void UrQMD::doInteraction(TView& view) {

    auto projectile = view.getProjectile();

    auto projectileCode = projectile.getPID();
    auto const projectileEnergyLab = projectile.getEnergy();
    auto const& projectileMomentumLab = projectile.getMomentum();
    auto const& projectilePosition = projectile.getPosition();
    auto const projectileTime = projectile.getTime();

    // sample target particle
    auto const& mediumComposition =
        projectile.getNode()->getModelProperties().getNuclearComposition();
    auto const componentCrossSections = std::invoke([&]() {
      auto const& components = mediumComposition.getComponents();
      std::vector<CrossSectionType> crossSections;
      crossSections.reserve(components.size());

      for (auto const c : components) {
        crossSections.push_back(getCrossSection(projectile, c));
      }

      return crossSections;
    });

    auto const targetCode = mediumComposition.sampleTarget(componentCrossSections, RNG_);
    auto const targetA = get_nucleus_A(targetCode);
    auto const targetZ = get_nucleus_Z(targetCode);

    ::urqmd::inputs_.nevents = 1;
    ::urqmd::sys_.eos = 0; // could be configurable in principle
    ::urqmd::inputs_.outsteps = 1;
    ::urqmd::sys_.nsteps = 1;

    // initialization regarding projectile
    if (Code::Nucleus == projectileCode) {
      // is this everything?
      ::urqmd::inputs_.prspflg = 0;

      ::urqmd::sys_.Ap = projectile.getNuclearA();
      ::urqmd::sys_.Zp = projectile.getNuclearZ();
      ::urqmd::rsys_.ebeam = (projectileEnergyLab - projectile.getMass()) * (1 / 1_GeV) /
                             projectile.getNuclearA();

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
      ::urqmd::rsys_.ebeam = (projectileEnergyLab - projectile.getMass()) * (1 / 1_GeV);

      if (projectileCode == Code::K0Long) {
        projectileCode = booleanDist_(RNG_) ? Code::K0 : Code::K0Bar;
      } else if (projectileCode == Code::K0Short) {
        throw std::runtime_error("K0Short should not interact");
      }

      auto const [ityp, iso3] = convertToUrQMD(projectileCode);
      // todo: conversion of K_long/short into strong eigenstates;
      ::urqmd::inputs_.spityp[0] = ityp;
      ::urqmd::inputs_.spiso3[0] = iso3;
    }

    // initilazation regarding target
    if (is_nucleus(targetCode)) {
      ::urqmd::sys_.Zt = targetZ;
      ::urqmd::sys_.At = targetA;
      ::urqmd::inputs_.trspflg = 0; // nucleus as target
      int const id = 2;
      ::urqmd::cascinit_(::urqmd::sys_.Zt, ::urqmd::sys_.At, id);
    } else {
      ::urqmd::inputs_.trspflg = 1; // special particle as target
      auto const [ityp, iso3] = convertToUrQMD(targetCode);
      ::urqmd::inputs_.spityp[1] = ityp;
      ::urqmd::inputs_.spiso3[1] = iso3;
    }

    int iflb = 0; // flag for retrying interaction in case of empty event, 0 means retry
    ::urqmd::urqmd_(iflb);

    // now retrieve secondaries from UrQMD
    auto const& originalCS = projectileMomentumLab.getCoordinateSystem();
    CoordinateSystemPtr const& zAxisFrame =
        make_rotationToZ(originalCS, projectileMomentumLab);

    for (int i = 0; i < ::urqmd::sys_.npart; ++i) {
      auto code = convertFromUrQMD(::urqmd::isys_.ityp[i], ::urqmd::isys_.iso3[i]);
      if (code == Code::K0 || code == Code::K0Bar) {
        code = booleanDist_(RNG_) ? Code::K0Short : Code::K0Long;
      }

      // "coor_.p0[i] * 1_GeV" is likely off-shell as UrQMD doesn't preserve masses well
      auto momentum =
          Vector(zAxisFrame,
                 QuantityVector<dimensionless_d>{
                     ::urqmd::coor_.px[i], ::urqmd::coor_.py[i], ::urqmd::coor_.pz[i]} *
                     1_GeV);

      auto const energy = sqrt(momentum.getSquaredNorm() + square(get_mass(code)));

      momentum.rebase(originalCS); // transform back into standard lab frame
      CORSIKA_LOG_DEBUG(" {} {} {} ", i, code, momentum.getComponents());

      projectile.addSecondary(
          std::make_tuple(code, energy, momentum, projectilePosition, projectileTime));
    }
    CORSIKA_LOG_DEBUG("UrQMD generated {} secondaries!", ::urqmd::sys_.npart);
  }

  inline Code convertFromUrQMD(int vItyp, int vIso3) {
    int const pdgInt =
        ::urqmd::pdgid_(vItyp, vIso3); // use the conversion function provided by UrQMD
    if (pdgInt == 0) {                 // ::urqmd::pdgid_ returns 0 on error
      throw std::runtime_error("UrQMD pdgid() returned 0");
    }
    auto const pdg = static_cast<PDGCode>(pdgInt);
    return convert_from_PDG(pdg);
  }

  inline std::pair<int, int> convertToUrQMD(Code code) {
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
