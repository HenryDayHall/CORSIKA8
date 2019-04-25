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
using SetupParticle = corsika::setup::Stack::StackIterator;
using SetupProjectile = corsika::setup::StackView::StackIterator;
using SetupTrack = corsika::setup::Trajectory;

template <typename TParticle> // need template here, as this is called both with
                              // SetupParticle as well as SetupProjectile
                              CrossSectionType UrQMD::GetCrossSection(
                                  TParticle const& vProjectile,
                                  corsika::particles::Code vTargetCode,
                                  corsika::units::si::HEPEnergyType vProjectileEnergyLab)
                                  const {
  using namespace units::si;

  // TODO: energy cuts, return 0 for non-hadrons

  auto const projectileCode = vProjectile.GetPID();

  // the following is a translation of ptsigtot() into C++
  if (projectileCode != particles::Code::Nucleus &&
      !IsNucleus(vTargetCode)) { // both particles are "special"
    auto const mProj = particles::GetMass(projectileCode);
    auto const mTar = particles::GetMass(vTargetCode);
    double sqrtS = sqrt(detail::static_pow<2>(mProj) + detail::static_pow<2>(mTar) +
                        2 * vProjectileEnergyLab * mTar) *
                   (1 / 1_GeV);

    // we must set some UrQMD globals first...
    auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
    inputs_.spityp[0] = ityp;
    inputs_.spiso3[0] = iso3;

    auto const [itypTar, iso3Tar] = ConvertToUrQMD(vTargetCode);
    inputs_.spityp[1] = itypTar;
    inputs_.spiso3[1] = iso3Tar;

    int one = 1;
    int two = 2;
    return sigtot_(one, two, sqrtS) * 1_mb;
  } else {
    int const Ap =
        (projectileCode == particles::Code::Nucleus) ? vProjectile.GetNuclearA() : 1;
    int const At = IsNucleus(vTargetCode) ? particles::GetNucleusA(vTargetCode) : 1;

    double const maxImpact = nucrad_(Ap) + nucrad_(At) + 2 * options_.CTParam[30 - 1];
    return 10_mb * M_PI * detail::static_pow<2>(maxImpact);
    // is a constant cross-section really reasonable?
  }
}

GrammageType UrQMD::GetInteractionLength(SetupParticle& vParticle, SetupTrack&) const {
  auto const& mediumComposition =
      vParticle.GetNode()->GetModelProperties().GetNuclearComposition();
  using namespace std::placeholders;

  CrossSectionType const weightedProdCrossSection = mediumComposition.WeightedSum(
      std::bind(&UrQMD::GetCrossSection<decltype(vParticle)>, this, vParticle, _1,
                vParticle.GetEnergy()));

  return mediumComposition.GetAverageMassNumber() * units::constants::u /
         weightedProdCrossSection;
}

corsika::process::EProcessReturn UrQMD::DoInteraction(SetupProjectile& vProjectile) {
  using namespace units::si;

  auto const projectileCode = vProjectile.GetPID();
  auto const projectileEnergyLab = vProjectile.GetEnergy();
  auto const& projectileMomentumLab = vProjectile.GetMomentum();
  auto const& projectilePosition = vProjectile.GetPosition();
  auto const projectileTime = vProjectile.GetTime();

  // sample target particle
  auto const& mediumComposition =
      vProjectile.GetNode()->GetModelProperties().GetNuclearComposition();
  auto const componentCrossSections = std::invoke([&]() {
    auto const& components = mediumComposition.GetComponents();
    std::vector<CrossSectionType> crossSections;
    crossSections.reserve(components.size());

    for (auto const c : components) {
      crossSections.push_back(GetCrossSection(vProjectile, c, projectileEnergyLab));
    }

    return crossSections;
  });

  auto const targetCode = mediumComposition.SampleTarget(componentCrossSections, fRNG);
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

    sys_.Ap = vProjectile.GetNuclearA();
    sys_.Zp = vProjectile.GetNuclearZ();

    rsys_.bdist = nucrad_(targetA) + nucrad_(sys_.Ap) + 2 * options_.CTParam[30 - 1];

    int const id = 1;
    cascinit_(sys_.Zp, sys_.Ap, id);
  } else {
    inputs_.prspflg = 1;
    sys_.Ap = 1; // even for non-baryons this has to be set, see vanilla UrQMD.f
    rsys_.bdist = nucrad_(targetA) + nucrad_(1) + 2 * options_.CTParam[30 - 1];

    auto const [ityp, iso3] = ConvertToUrQMD(projectileCode);
    // todo: conversion of K_long/short into strong eigenstates;
    inputs_.spityp[0] = ityp;
    inputs_.spiso3[0] = iso3;
  }

  rsys_.ebeam = (projectileEnergyLab - vProjectile.GetMass()) * (1 / 1_GeV);

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
    auto const code = ConvertFromUrQMD(isys_.ityp[i], isys_.iso3[i]);
    auto const energy = coor_.p0[i] * 1_GeV;
    auto momentum = geometry::Vector(
        zAxisFrame,
        geometry::QuantityVector<dimensionless_d>{coor_.px[i], coor_.py[i], coor_.pz[i]} *
            1_GeV);

    momentum.rebase(originalCS); // transform back into standard lab frame
    std::cout << i << " " << code << " " << momentum.GetComponents() << std::endl;

    vProjectile.AddSecondary(
        std::tuple<particles::Code, HEPEnergyType, stack::MomentumVector, geometry::Point,
                   TimeType>{code, energy, momentum, projectilePosition, projectileTime});
  }

  std::cout << "UrQMD generated " << sys_.npart << " secondaries!" << std::endl;

  if (sys_.npart > 0) // delete only in case of inelastic collision, otherwise keep
    vProjectile.Delete();

  return process::EProcessReturn::eOk;
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
  int const pdgInt =
      pdgid_(vItyp, vIso3); // use the conversion function provided by UrQMD
  if (pdgInt == 0) {        // pdgid_ returns 0 on error
    throw std::runtime_error("UrQMD pdgid() returned 0");
  }
  auto const pdg = static_cast<particles::PDGCode>(pdgInt);
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
