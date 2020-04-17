/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/geometry/QuantityVector.hpp>
#include <corsika/framework/geometry/Vector.hpp>
#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/framework/core/PhysicalUnits.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <fstream>
#include <functional>
#include <random>
#include <sstream>

using namespace corsika::UrQMD;
using namespace corsika::units::si;

UrQMD::UrQMD() { iniurqmd_(); }

using SetupStack = corsika::Stack;
using SetupParticle = corsika::Stack::StackIterator;
using SetupProjectile = corsika::StackView::StackIterator;

CrossSectionType UrQMD::GetCrossSection(particles::Code vProjectileCode,
                                        corsika::Code vTargetCode,
                                        HEPEnergyType vLabEnergy, int vAProjectile = 1) {
  // the following is a translation of ptsigtot() into C++
  if (vProjectileCode != particles::Code::Nucleus &&
      !IsNucleus(vTargetCode)) { // both particles are "special"
    auto const mProj = particles::GetMass(vProjectileCode);
    auto const mTar = particles::GetMass(vTargetCode);
    double sqrtS = sqrt(units::si::detail::static_pow<2>(mProj) +
                        units::si::detail::static_pow<2>(mTar) + 2 * vLabEnergy * mTar) *
                   (1 / 1_GeV);

    // we must set some UrQMD globals first...
    auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
    inputs_.spityp[0] = ityp;
    inputs_.spiso3[0] = iso3;

    auto const [itypTar, iso3Tar] = ConvertToUrQMD(targetCode);
    inputs_.spityp[1] = itypTar;
    inputs_.spiso3[1] = iso3Tar;

    int one = 1;
    int two = 2;
    int three = 3;

    double const totalXS = sigtot_(one, two, sqrtS);

    // subtract elastic cross-section as in ptsigtot()
    int itypmn, itypmx, iso3mn, iso3mx;
    if (ityp < itypTar) {
      itypmn = ityp;
      itypmx = itypTar;

      iso3mn = iso3;
      iso3mx = iso3Tar;
    } else {
      itypmx = ityp;
      itypmn = itypTar;

      iso3mx = iso3;
      iso3mn = iso3Tar;
    }

    int isigline = collclass_(itypmx, iso3mx, itypmn, iso3mn);
    int iline = readsigmaln_(three, one, isigline);
    double sigEl;
    double massProj = mProj / 1_GeV;
    double massTar = mTar / 1_GeV;

    crossx_(iline, sqrtS, ityp, iso3, massProj, itypTar, iso3Tar, massTar, sigEl);

    if (totalXS > sigEl) {
      return (totalXS - sigEl) * 1_mb;
    } else {
      return sigEl * 0_mb;
    }
  } else {
    int const Ap = projectileA;
    int const At = IsNucleus(targetCode) ? particles::GetNucleusA(targetCode) : 1;

    double const maxImpact = nucrad_(Ap) + nucrad_(At) + 2 * options_.CTParam[30 - 1];
    return 10_mb * M_PI * units::static_pow<2>(maxImpact);
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

  if (projectileCode == particles::Code::K0Long) {
    return 0.5 *
           (GetCrossSection(particles::Code::K0, vTargetCode, projectileEnergyLab) +
            GetCrossSection(particles::Code::K0Bar, vTargetCode, projectileEnergyLab));
  }

  return GetTabulatedCrossSection(projectileCode, targetCode, projectileEnergyLab);
}

bool UrQMD::CanInteract(particles::Code code) const {
  // According to the manual, UrQMD can use all mesons, baryons and nucleons
  // which are modeled also as input particles. I think it is safer to accept
  // only the usual long-lived species as input.

  // Interactions with nucleus projectiles are possible in principle with UrQMD
  // but right now we don't have access to the inelastic (production) cross-section,
  // so we unfortunately have to forbid these interactions for the time being.

  static particles::Code const validProjectileCodes[] = {
      particles::Code::Proton,      particles::Code::AntiProton, particles::Code::Neutron,
      particles::Code::AntiNeutron, particles::Code::PiPlus,     particles::Code::PiMinus,
      particles::Code::KPlus,       particles::Code::KMinus,     particles::Code::K0Short,
      particles::Code::K0Long};

  return std::find(std::cbegin(validProjectileCodes), std::cend(validProjectileCodes),
                   code) != std::cend(validProjectileCodes);
}

GrammageType UrQMD::GetInteractionLength(SetupParticle const& particle) const {
  if (!CanInteract(particle.GetPID())) {
    // we could do the canInteract check in GetCrossSection, too but if
    // we do it here we have the advantage of avoiding the loop
    return std::numeric_limits<double>::infinity() * 1_g / (1_cm * 1_cm);
  }

  auto const& mediumComposition =
      particle.GetNode()->GetModelProperties().GetNuclearComposition();
  using namespace std::placeholders;

  CrossSectionType const weightedProdCrossSection = mediumComposition.WeightedSum(
      std::bind(&UrQMD::GetCrossSection<decltype(particle)>, this, particle, _1));

  return mediumComposition.GetAverageMassNumber() * units::constants::u /
         weightedProdCrossSection;
}

corsika::EProcessReturn UrQMD::DoInteraction(SetupProjectile& vProjectile) {
  using namespace units::si;

  auto const projectile = view.GetProjectile();

  auto projectileCode = projectile.GetPID();
  auto const projectileEnergyLab = projectile.GetEnergy();
  auto const& projectileMomentumLab = projectile.GetMomentum();
  auto const& projectilePosition = projectile.GetPosition();
  auto const projectileTime = projectile.GetTime();

  C8LOG_DEBUG("UrQMD::DoInteraction pid={} E={} GeV", projectileCode,
              projectileEnergyLab / 1_GeV);

  // sample target particle
  auto const& mediumComposition =
      projectile.GetNode()->GetModelProperties().GetNuclearComposition();
  auto const componentCrossSections = std::invoke([&]() {
    auto const& components = mediumComposition.GetComponents();
    std::vector<CrossSectionType> crossSections;
    crossSections.reserve(components.size());

    for (auto const c : components) {
      crossSections.push_back(GetCrossSection(projectile, c));
    }

    return crossSections;
  });

  auto const targetCode = mediumComposition.SampleTarget(componentCrossSections, rng_);
  auto const targetA = particles::GetNucleusA(targetCode);
  auto const targetZ = particles::GetNucleusZ(targetCode);

  inputs_.nevents = 1;
  sys_.eos = 0; // could be configurable in principle
  inputs_.outsteps = 1;
  sys_.nsteps = 1;

  // initialization regarding projectile
  if (particles::Code::Nucleus == projectileCode) {
    // is this everything?
    inputs_.prspflg = 0;

    sys_.Ap = projectile.GetNuclearA();
    sys_.Zp = projectile.GetNuclearZ();
    rsys_.ebeam = (projectileEnergyLab - projectile.GetMass()) * (1 / 1_GeV) /
                  projectile.GetNuclearA();

    rsys_.bdist = nucrad_(targetA) + nucrad_(sys_.Ap) + 2 * options_.CTParam[30 - 1];

    int const id = 1;
    cascinit_(sys_.Zp, sys_.Ap, id);
  } else {
    inputs_.prspflg = 1;
    sys_.Ap = 1; // even for non-baryons this has to be set, see vanilla UrQMD.f
    rsys_.bdist = nucrad_(targetA) + nucrad_(1) + 2 * options_.CTParam[30 - 1];
    rsys_.ebeam = (projectileEnergyLab - projectile.GetMass()) * (1 / 1_GeV);

    if (projectileCode == particles::Code::K0Long ||
        projectileCode == particles::Code::K0Short) {
      projectileCode = booleanDist_(rng_) ? particles::Code::K0 : particles::Code::K0Bar;
    }

    auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
    // todo: conversion of K_long/short into strong eigenstates;
    inputs_.spityp[0] = ityp;
    inputs_.spiso3[0] = iso3;
  }

  // initilazation regarding target
  if (particles::IsNucleus(targetCode)) {
    sys_.Zt = targetZ;
    sys_.At = targetA;
    inputs_.trspflg = 0; // nucleus as target
    int const id = 2;
    cascinit_(sys_.Zt, sys_.At, id);
  } else {
    inputs_.trspflg = 1; // special particle as target
    auto const [ityp, iso3] = ConvertToUrQMD(targetCode);
    inputs_.spityp[1] = ityp;
    inputs_.spiso3[1] = iso3;
  }

  int iflb = 0; // flag for retrying interaction in case of empty event, 0 means retry
  urqmd_(iflb);

  // now retrieve secondaries from UrQMD
  auto const& originalCS = projectileMomentumLab.GetCoordinateSystem();
  geometry::CoordinateSystem const zAxisFrame =
      originalCS.RotateToZ(projectileMomentumLab);

  for (int i = 0; i < sys_.npart; ++i) {
    auto code = ConvertFromUrQMD(isys_.ityp[i], isys_.iso3[i]);
    if (code == particles::Code::K0 || code == particles::Code::K0Bar) {
      code = booleanDist_(rng_) ? particles::Code::K0Short : particles::Code::K0Long;
    }

    // "coor_.p0[i] * 1_GeV" is likely off-shell as UrQMD doesn't preserve masses well
    auto momentum = geometry::Vector(
        zAxisFrame,
        geometry::QuantityVector<dimensionless_d>{coor_.px[i], coor_.py[i], coor_.pz[i]} *
            1_GeV);

    auto const energy = sqrt(momentum.squaredNorm() + square(particles::GetMass(code)));

    momentum.rebase(originalCS); // transform back into standard lab frame
    C8LOG_DEBUG(" Secondary {} code {} p={} GeV", i, code,
                momentum.GetComponents() / 1_GeV);

    view.AddSecondary(
        std::make_tuple(code, energy, momentum, projectilePosition, projectileTime));
  }

  C8LOG_DEBUG("UrQMD generated {} secondaries!", sys_.npart);

  return process::EProcessReturn::eOk;
}

/**
 * the random number generator function of UrQMD
 */
double corsika::UrQMD::ranf_(int&) {
  static corsika::RNG& rng =
      corsika::RNGManager::GetInstance().GetRandomStream("UrQMD");
  static std::uniform_real_distribution<double> dist;

  return dist(rng);
}

corsika::Code corsika::UrQMD::ConvertFromUrQMD(int vItyp, int vIso3) {
  int const pdgInt =
      pdgid_(vItyp, vIso3); // use the conversion function provided by UrQMD
  if (pdgInt == 0) {        // pdgid_ returns 0 on error
    throw std::runtime_error("UrQMD pdgid() returned 0");
  }
  auto const pdg = static_cast<particles::PDGCode>(pdgInt);
  return particles::ConvertFromPDG(pdg);
}

std::pair<int, int> corsika::UrQMD::ConvertToUrQMD(
    corsika::Code code) {
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

  return mapPDGToUrQMD.at(static_cast<int>(GetPDG(code)));
}

void UrQMD::readXSFile(std::string const& filename) {
  std::ifstream file(filename, std::ios::in);

  if (!file.is_open()) { throw std::runtime_error(filename + " could not be opened."); }

  std::string line;

  std::getline(file, line);
  std::stringstream ss(line);

  char dummy;
  int nTargets, nProjectiles, nSupports;
  ss >> dummy >> nTargets >> nProjectiles >> nSupports;

  decltype(xs_interp_support_table_)::extent_gen extents;
  xs_interp_support_table_.resize(extents[nProjectiles][nTargets][nSupports]);

  for (int i = 0; i < nTargets; ++i) {
    for (int j = 0; j < nProjectiles; ++j) {
      for (int k = 0; k < nSupports; ++k) {
        std::getline(file, line);
        std::stringstream s(line);
        double energy, sigma;
        s >> energy >> sigma;
        xs_interp_support_table_[j][i][k] = sigma * 1_mb;
      }

      std::getline(file, line);
      std::getline(file, line);
    }
  }
}
