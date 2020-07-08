/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/conex_source_cut/CONEXSourceCut.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalConstants.h>
#include <algorithm>
#include <iomanip>
#include <utility>

using namespace corsika::process::conex_source_cut;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace corsika::setup;

corsika::process::EProcessReturn CONEXSourceCut::DoSecondaries(
    corsika::setup::StackView& vS) {
  auto p = vS.begin();
  HEPEnergyType const energy = p.GetEnergy();

  while (p != vS.end()) {
    Code const pid = p.GetPID();

    auto const it = std::find_if(egs_em_codes_.cbegin(), egs_em_codes_.cend(),
                                 [=](auto const& p) { return pid == p.first; });
    if (it == egs_em_codes_.cend()) {
      continue; // no EM particle
    }

    auto const& position = p.GetPosition();
    auto const direction = p.GetMomentum().normalized();

    int egs_pid = it->second;

    double E =
        energy / 1_MeV; // total energy, TODO: check if maybe kinetic should be used

    auto coords = position.GetCoordinates(conexObservationCS_) / 1_m;
    double x = coords[0].magnitude();
    double y = coords[1].magnitude();

    double altitude = ((position - center_).norm() - conex::earthRadius) / 1_m;

    double slantDistance =
        (position - showerAxis_.GetStart()).dot(showerAxis_.GetDirection()) / 1_m;

    // lateral coordinates in CONEX shower frame
    auto const d = position - showerAxis_.GetStart();
    auto const dShowerPlane = d - d.parallelProjectionOnto(showerAxis_.GetDirection());
    double lateralX = dShowerPlane.dot(x_sf_) / 1_m;
    double lateralY = dShowerPlane.dot(y_sf_) / 1_m;

    double slantX = showerAxis_.projectedX(position) * (1_cm * 1_cm / 1_g);

    double time = (p.GetTime() * units::constants::c - groundDist_) / 1_m;

    // fill u,v,w momentum direction in EGS frame
    double u = direction.dot(y_sf_).magnitude();
    double v = direction.dot(x_sf_).magnitude();
    double w = direction.dot(showerAxis_.GetDirection()).magnitude();

    int iri = 2; // EGS medium air

    double weight = 1;

    int latchin = 1; // generation, we don't have the actual value...

    std::cout << "CONEXSourceCut: removing " << pid << " " << std::scientific << energy
              << std::endl;
    p.Delete();

    //conex_.Shower(egs_pid, E, x, y, altitude, slantDistance, lateralX, lateralY, slantX,
    //              time, u, v, w, iri, weight, latchin);
    conex::show_(egs_pid, E, x, y, altitude, slantDistance, lateralX, lateralY, slantX,
		 time, u, v, w, iri, weight, latchin);
  }

  return corsika::process::EProcessReturn::eOk;
}

void CONEXSourceCut::Init() {}

void CONEXSourceCut::SolveCE() {
  int zero = 0;
  int iCEmode = 1;
  int id = 0;      // RU: max, fix this
  int nshtot_ = 0; // RU: max, fix this
  //conex_.HadronCascade(id, nshtot_, zero, iCEmode);
  //conex_.SolveMomentEquations(zero);
  conex::hadroncascade_(id, nshtot_, zero, iCEmode);
  conex::solvemomentequations_(zero);

  // RU: this here is from cxroot,

  //int nX = conex_.GetNumberOfDepthBins(); // make sure this works!
  int nX = conex::get_number_of_depth_bins_(); // make sure this works!

  int icut = 1;
  int icutg = 2;
  int icute = 3;
  int icutm = 2;
  int icuth = 3;
  int iSec = 0;

  const int maxX = nX;

  auto X = std::make_unique<float[]>(maxX);
  auto H = std::make_unique<float[]>(maxX);
  auto D = std::make_unique<float[]>(maxX);
  auto N = std::make_unique<float[]>(maxX);
  auto dEdX = std::make_unique<float[]>(maxX);
  auto Mu = std::make_unique<float[]>(maxX);
  auto dMu = std::make_unique<float[]>(maxX);
  auto Gamma = std::make_unique<float[]>(maxX);
  auto Electrons = std::make_unique<float[]>(maxX);
  auto Hadrons = std::make_unique<float[]>(maxX);

  float EGround[3], fitpars[13], currlgE, Xmx, Nmx, XmxdEdX, dEdXmx;

  // conex_.GetShowerData(icut, iSec, nX, X[0], N[0], fitpars[0], H[0], D[0]);
  // conex_.GetdEdXProfile(icut, nX, dEdX[0], EGround[0]);
  // conex_.GetMuonProfile(icutm, nX, Mu[0], dMu[0]);
  // conex_.GetGammaProfile(icutg, nX, Gamma[0]);
  // conex_.GetElectronProfile(icute, nX, Electrons[0]);
  // conex_.GetHadronProfile(icuth, nX, Hadrons[0]);
   conex::get_shower_data_(icut, iSec, nX, X[0], N[0], fitpars[0], H[0], D[0]);
   conex::get_shower_edep_(icut, nX, dEdX[0], EGround[0]);
   conex::get_shower_muon_(icutm, nX, Mu[0], dMu[0]);
   conex::get_shower_gamma_(icutg, nX, Gamma[0]);
   conex::get_shower_electron_(icute, nX, Electrons[0]);
   conex::get_shower_hadron_(icuth, nX, Hadrons[0]);
}

// RU: move all the non-C8 code from the following c++ function into a new file. Here we
// only want to have a single function call to CONEX left.

CONEXSourceCut::CONEXSourceCut(geometry::Point center, environment::ShowerAxis showerAxis,
                               units::si::LengthType groundDist,
                               // units::si::GrammageType Xcut,
                               units::si::HEPEnergyType primaryEnergy,
                               particles::PDGCode primaryID)
    : //conex_{ConexDynamicInterface(eSibyll23)}
      center_{center}
    , showerAxis_{showerAxis}
    , groundDist_{groundDist}
    , conexObservationCS_{std::invoke([&]() {
      auto const& c8cs = center.GetCoordinateSystem();
      auto const showerCore =
          showerAxis.GetStart() + showerAxis.GetDirection() * groundDist;
      auto const translation = showerCore - center;
      auto const intermediateCS = c8cs.translate(translation.GetComponents(c8cs));
      auto const intermediateCS2 = intermediateCS.RotateToZ(translation);

      auto const transform = geometry::CoordinateSystem::GetTransformation(
          c8cs, intermediateCS2); // either this way or vice versa... TODO: test this!
      return geometry::CoordinateSystem(c8cs, transform);
    })}
    , x_sf_{std::invoke([&]() {
      return geometry::Vector<length_d>{conexObservationCS_, 0._m, 0._m, 1._m}
          .cross(showerAxis_.GetDirection())
          .normalized();
    })}
    , y_sf_{showerAxis_.GetDirection().cross(x_sf_)} {

  int randomSeeds[3] = {1234, 0, 0}; // will be overwritten later??
  int heModel = eSibyll23;
  
  int nShower = 1; // large to avoid final stats.
  int maxDetail = 0;
  int particleListMode = 0;
  //conex_.Init(nShower, randomSeeds, maxDetail, particleListMode, parameterPathName);

  std::string configPath = CONEX_CONFIG_PATH;
  conex::initconex_(nShower, randomSeeds,
		    heModel,
		    maxDetail,
#ifdef CONEX_EXTENSIONS
		    particleListMode,
#endif
		    configPath.c_str(),
		    configPath.size());

  double eprima = primaryEnergy / 1_GeV;

  // set phi, theta
  geometry::Vector<length_d> ez{conexObservationCS_, {0._m, 0._m, 1_m}};
  auto const c = showerAxis_.GetDirection().dot(ez) / 1_m;
  double theta = 180 * (M_PI - std::acos(c));

  auto const showerAxisConex =
      showerAxis_.GetDirection().GetComponents(conexObservationCS_);
  double phi = 180 * std::atan2(-showerAxisConex.GetY().magnitude(),
                                showerAxisConex.GetX().magnitude());
  // double XmaxP_ = Xcut / (1_g / 1_cm / 1_cm);

  int ipart = static_cast<int>(primaryID);
  auto rng = corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");

  double dimpact = 0.; // valid only if shower core is fixed on the observation plane; for
                       // skimming showers an offset is needed like in CONEX

  std::array<int, 3> ioseed{static_cast<int>(rng()), static_cast<int>(rng()),
                            static_cast<int>(rng())};

  //conex_.ConexRun(ipart, eprima, theta, phi, dimpact, ioseed.data());
  conex::conexrun_(ipart, eprima, theta, phi, dimpact, ioseed.data());
}
