#include <corsika/geometry/QuantityVector.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/process/urqmd/UrQMD.h>
#include <corsika/random/RNGManager.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

#include <functional>
#include <iostream>
#include <random>

using namespace corsika::process::UrQMD;
using namespace corsika::units::si;

UrQMD::UrQMD() { iniurqmd_(); }

using SetupStack = corsika::setup::Stack;
using SetupParticle = SetupStack::StackIterator;
using SetupTrack = corsika::setup::Trajectory;

CrossSectionType UrQMD::GetCrossSection(
    corsika::particles::Code projectileID, corsika::particles::Code targetID,
    corsika::units::si::HEPEnergyType projectileEnergy) const {
  using namespace units::si;
  return 10_mb; // TODO: implement
}

template <>
GrammageType UrQMD::GetInteractionLength(SetupParticle& particle, SetupTrack&) const {
  auto const& mediumComposition =
      particle.GetNode()->GetModelProperties().GetNuclearComposition();
  using namespace std::placeholders;

  CrossSectionType const weightedProdCrossSection =
      mediumComposition.WeightedSum(std::bind(
          &UrQMD::GetCrossSection, this, particle.GetPID(), _1, particle.GetEnergy()));

  return mediumComposition.GetAverageMassNumber() * units::constants::u /
         weightedProdCrossSection;
}

template <>
corsika::process::EProcessReturn UrQMD::DoInteraction(SetupParticle& projectile,
                                                      SetupStack&) {
  using namespace units::si;

  auto const projectileCode = projectile.GetPID();
  auto const projectileEnergyLab = projectile.GetEnergy();
  auto const& projectileMomentumLab = projectile.GetMomentum();
  auto const& projectilePosition = projectile.GetPosition();
  auto const projectileTime = projectile.GetTime();

  inputs_.nevents = 1;
  sys_.eos = 0; // could be configurable in principle
  inputs_.outsteps = 1;
  sys_.nsteps = 1;

  // todo: sample target

  int const Atarget = 14;
  int tableIndex = 0; // 0: nitrogen, 1: oxygen, 2: argon target

  /*
  // corsika 7
        bmin    = 0.d0
        CTOption(5) = 1
        if ( iflbmax.eq.1 ) then
          bdist       = BIM(LIT)
        else
          bdist=nucrad(Ap)+nucrad(At)+2*CTParam(30)
        endif

  // conex
        CTOption(5)=1
        if ( prspflg.eq.0 ) then
          bdist = BIM(LT)
        else
          bdist = xsbmax
        endif
  */

  // initialization regarding projectile
  if (particles::Code::Nucleus == projectileCode) {
    // is this everything?
    inputs_.prspflg = 0;
    rsys_.bdist = cxs_u2_.bim[tableIndex];

    sys_.Ap = projectile.GetNuclearA();
    sys_.Zp = projectile.GetNuclearZ();

    int const id = 1;
    cascinit_(sys_.Zp, sys_.Ap, id);
  } else {
    inputs_.prspflg = 1;
    int const Ap =
        1; // what value to use here for non-baryons??? see CONEX UrQMD interface
    rsys_.bdist = nucrad_(Ap) + nucrad_(Atarget) + 2 * options_.CTParam[30 - 1];

    auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
    // todo: conversion of K_long/short into strong eigenstates;
    inputs_.spityp[0] = ityp;
    inputs_.spiso3[0] = iso3;
  }

  rsys_.ebeam = (projectileEnergyLab - particles::GetMass(projectileCode)) * (1 / 1_GeV);

  // initilazation regarding target
  auto const& mediumComposition =
      projectile.GetNode()->GetModelProperties().GetNuclearComposition();
  auto const componentCrossSections = std::invoke([&]() {
    auto const& components = mediumComposition.GetComponents();
    std::vector<CrossSectionType> crossSections;
    crossSections.reserve(components.size());

    for (auto const c : components) {
      crossSections.push_back(GetCrossSection(projectileCode, c, projectileEnergyLab));
    }

    return crossSections;
  });

  auto const targetCode = mediumComposition.SampleTarget(componentCrossSections, fRNG);

  if (particles::IsNucleus(targetCode)) {
    sys_.Zt = particles::GetNucleusZ(targetCode);
    sys_.At = particles::GetNucleusA(targetCode);
    inputs_.trspflg = 0; // nucleus as target
    int const id = 2;
    cascinit_(sys_.Zt, sys_.At, id);
  } else {
    inputs_.trspflg = 1; // special particle as target
    auto const [ityp, iso3] = ConvertToUrQMD(targetCode);
    inputs_.spityp[1] = ityp;
    inputs_.spiso3[1] = iso3;
  }

  int iflb = 0; // flag for retrying interaction in case of elastic scattering
  urqmd_(iflb);

  // now retrieve secondaries from UrQMD
  auto const& originalCS = projectileMomentumLab.GetCoordinateSystem();
  geometry::CoordinateSystem const zAxisFrame =
      originalCS.RotateToZ(projectileMomentumLab);

  for (int i = 0; i < sys_.npart; ++i) {
    auto const code = ConvertFromUrQMD(isys_.ityp[i], isys_.iso3[i]);
    auto const energy = coor_.p0[i] * 1_GeV;
    auto momentum = geometry::Vector(
        zAxisFrame,
        geometry::QuantityVector<dimensionless_d>{coor_.px[i], coor_.py[i], coor_.pz[i]} *
            1_GeV);

    momentum.rebase(originalCS); // transform back into standard lab frame

    projectile.AddSecondary(
        std::tuple<particles::Code, HEPEnergyType, stack::MomentumVector, geometry::Point,
                   TimeType>{code, energy, momentum, projectilePosition, projectileTime});
  }

  if (sys_.npart > 0) // delete only in case of inelastic collision, otherwise keep
    projectile.Delete();
}

/**
 * the random number generator function of UrQMD
 */
double corsika::process::UrQMD::ranf_(int&) {
  static corsika::random::RNG& rng =
      corsika::random::RNGManager::GetInstance().GetRandomStream("UrQMD");
  static std::uniform_real_distribution<double> dist;

  return dist(rng);
}

corsika::particles::Code corsika::process::UrQMD::ConvertFromUrQMD(int vItyp, int vIso3) {
  static const std::map<std::pair<int, int>, int> mapUrQMDToPDG{
      // data from github.com/afedynitch/ParticleDataTool
      {{100, 0}, 22},      // gamma
      {{101, 0}, 111},     // pi0
      {{101, 2}, 211},     // pi+
      {{101, -2}, -211},   // pi-
      {{106, 1}, 321},     // K+
      {{106, -1}, -321},   // K-
      {{1, 1}, 2212},      // p
      {{1, -1}, 2112},     // n
      {{102, 0}, 221},     // eta
      {{104, 2}, 213},     // rho+
      {{104, -2}, -213},   // rho-
      {{104, 0}, 113},     // rho0
      {{108, 2}, 323},     // K*+
      {{108, -2}, -323},   // K*-
      {{108, 0}, 313},     // K*0
      {{-108, 0}, -313},   // K*0-bar
      {{103, 0}, 223},     // omega
      {{109, 0}, 333},     // phi
      {{40, 2}, 3222},     // Sigma+
      {{40, 0}, 3212},     // Sigma0
      {{40, -2}, 3112},    // Sigma-
      {{49, 0}, 3322},     // Xi0
      {{49, -1}, 3312},    // Xi-
      {{27, 0}, 3122},     // Lambda0
      {{17, 4}, 2224},     // Delta++
      {{17, 2}, 2214},     // Delta+
      {{17, 0}, 2114},     // Delta0
      {{17, -2}, 1114},    // Delta-
      {{41, 2}, 3224},     // Sigma*+
      {{41, 0}, 3214},     // Sigma*0
      {{41, -2}, 3114},    // Sigma*-
      {{50, 0}, 3324},     // Xi*0
      {{50, -1}, 3314},    // Xi*-
      {{55, 0}, 3334},     // Omega-
      {{133, 2}, 411},     // D+
      {{133, -2}, -411},   // D-
      {{133, 0}, 421},     // D0
      {{-133, 0}, -421},   // D0-bar
      {{107, 0}, 441},     // etaC
      {{138, 1}, 431},     // Ds+
      {{138, -1}, -431},   // Ds-
      {{139, 1}, 433},     // Ds*+
      {{139, -1}, -433},   // Ds*-
      {{134, 1}, 413},     // D*+
      {{134, -1}, -413},   // D*-
      {{134, 0}, 10421},   // D*0
      {{-134, 0}, -10421}, // D*0-bar
      {{135, 0}, 443},     // jpsi
  };

  auto const pdg = static_cast<particles::PDGCode>(mapUrQMDToPDG.at({vItyp, vIso3}));
  return particles::ConvertFromPDG(pdg);
}

std::pair<int, int> corsika::process::UrQMD::ConvertToUrQMD(
    corsika::particles::Code code) {
  static const std::map<int, std::pair<int, int>> mapPDGToUrQMD{
      // data from github.com/afedynitch/ParticleDataTool
      {22, {100, 0}},      // gamma
      {111, {101, 0}},     // pi0
      {211, {101, 2}},     // pi+
      {-211, {101, -2}},   // pi-
      {321, {106, 1}},     // K+
      {-321, {106, -1}},   // K-
      {2212, {1, 1}},      // p
      {2112, {1, -1}},     // n
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
