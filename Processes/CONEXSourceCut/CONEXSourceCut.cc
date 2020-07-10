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
#include <corsika/process/conex_source_cut/CONEX_f.h>
#include <corsika/random/RNGManager.h>
#include <corsika/units/PhysicalConstants.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <utility>

using namespace corsika::process::conex_source_cut;
using namespace corsika::units::si;
using namespace corsika::particles;
using namespace corsika::setup;

corsika::process::EProcessReturn CONEXSourceCut::DoSecondaries(
    corsika::setup::StackView& vS) {
  auto p = vS.begin();

  while (p != vS.end()) {
    Code const pid = p.GetPID();

    auto const it = std::find_if(egs_em_codes_.cbegin(), egs_em_codes_.cend(),
                                 [=](auto const& p) { return pid == p.first; });
    if (it == egs_em_codes_.cend()) {
      continue; // no EM particle
    }

    int egs_pid = it->second;

    addParticle(egs_pid, p.GetEnergy(), p.GetPosition(), p.GetMomentum().normalized(),
                p.GetTime());
  }

  return corsika::process::EProcessReturn::eOk;
}

void CONEXSourceCut::addParticle(int egs_pid, HEPEnergyType energy,
                                 geometry::Point const& position,
                                 geometry::Vector<dimensionless_d> const& direction,
                                 TimeType t) {
  std::cout << "position conexObs: " << position.GetCoordinates(conexObservationCS_)
            << std::endl;

  auto coords = position.GetCoordinates(conexObservationCS_) / 1_m;
  double x = coords[0].magnitude();
  double y = coords[1].magnitude();

  double altitude = ((position - center_).norm() - conex::earthRadius) / 1_m;
  auto const d = position - showerCore_;

  // distance from core to particle projected along shower axis
  double slantDistance = -d.dot(showerAxis_.GetDirection()) / 1_m;

  // lateral coordinates in CONEX shower frame
  auto const dShowerPlane = d - d.parallelProjectionOnto(showerAxis_.GetDirection());
  double lateralX = dShowerPlane.dot(x_sf_) / 1_m;
  double lateralY = dShowerPlane.dot(y_sf_) / 1_m;

  double slantX = showerAxis_.projectedX(position) * (1_cm * 1_cm / 1_g);

  double time = (t * units::constants::c - groundDist_) / 1_m;

  // fill u,v,w momentum direction in EGS frame
  double u = direction.dot(y_sf_).magnitude();
  double v = direction.dot(x_sf_).magnitude();
  double w = direction.dot(showerAxis_.GetDirection()).magnitude();

  int iri = 2; // EGS medium air

  double weight = 1;

  int latchin = 1; // generation, we don't have the actual value...
  double E = energy / 1_GeV;

  std::cout << "CONEXSourceCut: removing " << egs_pid << " " << std::scientific << energy
            << std::endl;

  // conex_.Shower(egs_pid, E, x, y, altitude, slantDistance, lateralX, lateralY,
  // slantX,
  //              time, u, v, w, iri, weight, latchin);

  std::cout << "#### parameters to show_() ####" << std::endl;
  std::cout << "egs_pid = " << egs_pid << std::endl;
  std::cout << "E = " << E << std::endl;
  std::cout << "x = " << x << std::endl;
  std::cout << "y = " << y << std::endl;
  std::cout << "altitude = " << altitude << std::endl;
  std::cout << "slantDistance = " << slantDistance << std::endl;
  std::cout << "lateralX = " << lateralX << std::endl;
  std::cout << "lateralY = " << lateralY << std::endl;
  std::cout << "slantX = " << slantX << std::endl;
  std::cout << "time = " << time << std::endl;
  std::cout << "u = " << u << std::endl;
  std::cout << "v = " << v << std::endl;
  std::cout << "w = " << w << std::endl;

  conex::cxoptl_.dptl[10 - 1] = egs_pid;
  conex::cxoptl_.dptl[4 - 1] = E;
  conex::cxoptl_.dptl[6 - 1] = x;
  conex::cxoptl_.dptl[7 - 1] = y;
  conex::cxoptl_.dptl[8 - 1] = altitude;
  conex::cxoptl_.dptl[9 - 1] = time;
  conex::cxoptl_.dptl[11 - 1] = weight;
  conex::cxoptl_.dptl[13 - 1] = slantX;
  conex::cxoptl_.dptl[14 - 1] = lateralX;
  conex::cxoptl_.dptl[15 - 1] = lateralY;
  conex::cxoptl_.dptl[16 - 1] = slantDistance;
  conex::cxoptl_.dptl[2 - 1] = u;
  conex::cxoptl_.dptl[1 - 1] = v;
  conex::cxoptl_.dptl[3 - 1] = w;

  int n = 0, i = 0;
  conex::cegs4_(n, i);
}

void CONEXSourceCut::Init() {}

void CONEXSourceCut::SolveCE() {
  int zero = 0;
  int iCEmode = 1;
  int id = 0;      // RU: max, fix this
  int nshtot_ = 0; // RU: max, fix this
  // conex_.HadronCascade(id, nshtot_, zero, iCEmode);
  // conex_.SolveMomentEquations(zero);
  conex::conexcascade_();

  // RU: this here is from cxroot,

  // int nX = conex_.GetNumberOfDepthBins(); // make sure this works!
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

  std::ofstream file{"conex_output.txt"};
  for (int i = 0; i < nX; ++i) {
    file << X[i] << " " << N[i] << " " << dEdX[i] << " " << Mu[i] << " " << dMu[i] << " "
         << Gamma[i] << " " << Electrons[i] << " " << Hadrons[i] << std::endl;
  }
}

// RU: move all the non-C8 code from the following c++ function into a new file. Here we
// only want to have a single function call to CONEX left.

CONEXSourceCut::CONEXSourceCut(geometry::Point center,
                               environment::ShowerAxis const& showerAxis,
                               units::si::LengthType groundDist,
                               units::si::LengthType injectionHeight,
                               units::si::HEPEnergyType primaryEnergy,
                               particles::PDGCode primaryID)
    : // conex_{ConexDynamicInterface(eSibyll23)}
    center_{center}
    , showerAxis_{showerAxis}
    , groundDist_{groundDist}
    , showerCore_{showerAxis_.GetStart() + showerAxis_.GetDirection() * groundDist_}
    , conexObservationCS_{std::invoke([&]() {
      auto const& c8cs = center.GetCoordinateSystem();
      auto const translation = showerCore_ - center;
      auto const intermediateCS = c8cs.translate(translation.GetComponents(c8cs));
      auto const intermediateCS2 = intermediateCS.RotateToZ(translation);

      std::cout << "translation C8/CONEX obs: " << translation.GetComponents()
                << std::endl;

      auto const transform = geometry::CoordinateSystem::GetTransformation(
          intermediateCS2, c8cs); // either this way or vice versa... TODO: test this!
      std::cout << transform.matrix() << std::endl << std::endl;
      std::cout
          << geometry::CoordinateSystem::GetTransformation(intermediateCS, c8cs).matrix()
          << std::endl
          << std::endl;
      std::cout << geometry::CoordinateSystem::GetTransformation(intermediateCS2,
                                                                 intermediateCS)
                       .matrix()
                << std::endl;

      return geometry::CoordinateSystem(c8cs, transform);
    })}
    , x_sf_{std::invoke([&]() {
      geometry::Vector<length_d> const a{conexObservationCS_, 0._m, 0._m, 1._m};
      auto b = a.cross(showerAxis_.GetDirection());
      auto const lengthB = b.norm();
      if (lengthB < 1e-10_m) {
        b = geometry::Vector<length_d>{conexObservationCS_, 1_m, 0_m, 0_m};
      }

      return b.normalized();
    })}
    , y_sf_{showerAxis_.GetDirection().cross(x_sf_)} {

  std::cout << "x_sf (conexObservationCS): " << x_sf_.GetComponents(conexObservationCS_)
            << std::endl;
  std::cout << "x_sf (C8): " << x_sf_.GetComponents(center.GetCoordinateSystem())
            << std::endl;

  std::cout << "y_sf (conexObservationCS): " << y_sf_.GetComponents(conexObservationCS_)
            << std::endl;
  std::cout << "y_sf (C8): " << y_sf_.GetComponents(center.GetCoordinateSystem())
            << std::endl;

  std::cout << "showerAxisDirection (conexObservationCS): "
            << showerAxis_.GetDirection().GetComponents(conexObservationCS_) << std::endl;
  std::cout << "showerAxisDirection (C8): "
            << showerAxis_.GetDirection().GetComponents(center.GetCoordinateSystem())
            << std::endl;

  std::cout << "showerCore (conexObservationCS): "
            << showerCore_.GetCoordinates(conexObservationCS_) << std::endl;
  std::cout << "showerCore (C8): "
            << showerCore_.GetCoordinates(center.GetCoordinateSystem()) << std::endl;

  int randomSeeds[3] = {1234, 0, 0}; // will be overwritten later??
  int heModel = eSibyll23;

  int nShower = 1; // large to avoid final stats.
  int maxDetail = 0;
  int particleListMode = 0;
  // conex_.Init(nShower, randomSeeds, maxDetail, particleListMode, parameterPathName);

  std::string configPath = CONEX_CONFIG_PATH;
  conex::initconex_(nShower, randomSeeds, heModel, maxDetail,
#ifdef CONEX_EXTENSIONS
                    particleListMode,
#endif
                    configPath.c_str(), configPath.size());

  double eprima = primaryEnergy / 1_GeV;

  // set phi, theta
  geometry::Vector<length_d> ez{conexObservationCS_, {0._m, 0._m, -1_m}};
  auto const c = showerAxis_.GetDirection().dot(ez) / 1_m;
  double theta = std::acos(c) * 180 / M_PI;

  auto const showerAxisConex =
      showerAxis_.GetDirection().GetComponents(conexObservationCS_);
  double phi = std::atan2(-showerAxisConex.GetY().magnitude(),
                          showerAxisConex.GetX().magnitude()) *
               180 / M_PI;

  std::cout << "theta (deg) = " << theta << "; phi (deg) = " << phi << std::endl;

  int ipart = static_cast<int>(primaryID);
  auto rng = corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");

  double dimpact = 0.; // valid only if shower core is fixed on the observation plane; for
                       // skimming showers an offset is needed like in CONEX

  std::array<int, 3> ioseed{static_cast<int>(rng()), static_cast<int>(rng()),
                            static_cast<int>(rng())};

  double xminp = injectionHeight / 1_m;

  // conex_.ConexRun(ipart, eprima, theta, phi, dimpact, ioseed.data());
  conex::conexrun_(ipart, eprima, theta, phi, xminp, dimpact, ioseed.data());
}
