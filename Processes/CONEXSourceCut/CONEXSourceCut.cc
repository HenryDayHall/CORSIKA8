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

    auto const it = std::find_if(em_codes_.cbegin(), em_codes_.cend(),
                                 [=](auto const& p) { return pid == p.first; });
    if (it == em_codes_.cend()) {
      continue; // no EM particle
    }

    auto const& position = p.GetPosition();
    auto const direction = p.GetMomentum().normalized();

    int egs_pid = it->second;

    double E =
        energy / 1_MeV; // total energy, TODO: check if maybe kinetic should be used

    auto coords = position.GetCoordinates(conexObservationCS_) / 1_m;
    double x = coords[0];
    double y = coords[1];

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
    double u = direction.dot(y_sf_);
    double v = direction.dot(x_sf_);
    double w = direction.dot(showerAxis_.GetDirection());

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

void CONEXSourceCut::SolveCE() {
  int zero = 0;
  int iCEmode = 1;
  conex::HadronCascade_(id, nshtot_, zero, iCEmode);
  conex::SolveMomentEquations_(zero);
}

CONEXSourceCut::CONEXSourceCut(geometry::Point center, environment::ShowerAxis showerAxis,
                               units::si::LengthType groundDist,
                               units::si::GrammageType Xcut,
                               units::si::EnergyType primaryEnergy)
    : center_{center}
    , showerAxis_{showerAxis}
    , groundDist_{groundDist}
    , conexObservation_{std::invoke([]() {
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
    , x_sf_{geometry::Vector<dimensionless_d>{conexObservationCS_, 0., 0., 1.}
                .cross(showerAxis_.GetDirection())
                .normalized()}
    , y_sf_{showerAxis_.GetDirection().cross(x_sf_)} {
  auto id = static_cast<int>(InitialParticle_(particles::GetPDG(pid)));
  conex::eprima_ = primaryEnergy / 1_GeV;
  nshtot = 1;                         // not sure about this...
  conex::fehcut_ = conex::femcut_ = ; // what?
  conex::ehcut_ = max(enymin, min(1.d10, eprima / aNbrNucl * fehcut));
  conex::emcut_ = max(enymin, min(1.d10, eprima / aNbrNucl * femcut));

  // set phisho, thetas
  geometry::Vector<dimensionless_d> ez{conexObservationCS_, 0., 0., 1.};
  auto const c = showerAxis_.GetDirection().dot(ez);
  auto const theta = M_PI - std::acos(c);
  conex::cxbas4_::thetas_ = theta * 180;

  auto const showerAxisConex =
      showerAxis_.GetDirection().GetComponents(conexObservationCS_);
  auto const phi = std::atan2(-showerAxisConex.GetY(), showerAxisConex.GetX());
  conex::cxbas4_::phisho_ = phi * 180;

  // call ranfgt(seed)?

  cxthin_::ethin = 0;

  conex::cxoutput1_::XminP_ = conex::cxoutput1_::XmaxP_ = Xcut / (1_g / 1_cm / 1_cm);
}
