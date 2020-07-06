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

    conex::show_(egs_pid, E, x, y, altitude, slantDistance, lateralX, lateralY, slantX,
                 time, u, v, w, iri, weight, latchin);
  }

  return corsika::process::EProcessReturn::eOk;
}

void CONEXSourceCut::Init() {}

void CONEXSourceCut::SolveCE() { conex::conexcascade_(); }

CONEXSourceCut::CONEXSourceCut(geometry::Point center, environment::ShowerAxis showerAxis,
                               units::si::LengthType groundDist,
                               units::si::GrammageType Xcut,
                               units::si::HEPEnergyType primaryEnergy,
                               particles::PDGCode primaryID)
    : center_{center}
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

  double eprima = primaryEnergy / 1_GeV;

  // set phi, theta
  geometry::Vector<length_d> ez{conexObservationCS_, {0._m, 0._m, 1_m}};
  auto const c = showerAxis_.GetDirection().dot(ez) / 1_m;
  double theta = 180 * (M_PI - std::acos(c));

  auto const showerAxisConex =
      showerAxis_.GetDirection().GetComponents(conexObservationCS_);
  double phi = 180 * std::atan2(-showerAxisConex.GetY().magnitude(),
                                showerAxisConex.GetX().magnitude());
  double XmaxP_ = Xcut / (1_g / 1_cm / 1_cm);

  int ipart = static_cast<int>(primaryID);
  auto rng = corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");

  double dimpact = 0.; // valid only if shower core is fixed on the observation plane; for
                       // skimming showers an offset is needed like in CONEX

  std::array<int, 3> ioseed{static_cast<int>(rng()), static_cast<int>(rng()),
                            static_cast<int>(rng())};

  conex::conexrun_(ipart, eprima, theta, phi, dimpact, ioseed.data());
}
