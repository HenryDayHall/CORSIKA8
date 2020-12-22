/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/logging/Logging.hpp>
#include <corsika/modules/conex/CONEXhybrid.hpp>
#include <corsika/modules/conex/CONEX_f.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/core/PhysicalConstants.hpp>

#include <conexConfig.h>

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <utility>

namespace corsika {

  CONEXhybrid::CONEXhybrid(Point center, ShowerAxis const& showerAxis,
                           LengthType groundDist, LengthType injectionHeight,
                           HEPEnergyType primaryEnergy, PDGCode primaryPDG)
      : center_{center}
      , showerAxis_{showerAxis}
      , groundDist_{groundDist}
      , showerCore_{showerAxis_.getStart() + showerAxis_.getDirection() * groundDist_}
      , conexObservationCS_{std::invoke([&]() {
        auto const& c8cs = center.getCoordinateSystem();
        auto const translation = showerCore_ - center;
        auto const intermediateCS =
            make_translation(c8cs, translation.getComponents(c8cs));
        auto const transformCS = make_rotationToZ(intermediateCS, translation);

        std::cout << "translation C8/CONEX obs: " << translation.getComponents()
                  << std::endl;

        /*
        auto const transform = CoordinateSystem::getTransformation(
            intermediateCS2, c8cs); // either this way or vice versa... TODO: test this!
        std::cout << transform.matrix() << std::endl << std::endl;
        std::cout << CoordinateSystem::getTransformation(intermediateCS, c8cs).matrix()
                  << std::endl
                  << std::endl;
        std::cout << CoordinateSystem::getTransformation(intermediateCS2, intermediateCS)
                         .matrix()
                  << std::endl;
        */
        return transformCS;
      })}
      , x_sf_{std::invoke([&]() {
        Vector<length_d> const a{conexObservationCS_, 0._m, 0._m, 1._m};
        auto b = a.cross(showerAxis_.getDirection());
        auto const lengthB = b.getNorm();
        if (lengthB < 1e-10_m) {
          b = Vector<length_d>{conexObservationCS_, 1_m, 0_m, 0_m};
        }

        return b.normalized();
      })}
      , y_sf_{showerAxis_.getDirection().cross(x_sf_)} {

    std::cout << "x_sf (conexObservationCS): " << x_sf_.getComponents(conexObservationCS_)
              << std::endl;
    std::cout << "x_sf (C8): " << x_sf_.getComponents(center.getCoordinateSystem())
              << std::endl;

    std::cout << "y_sf (conexObservationCS): " << y_sf_.getComponents(conexObservationCS_)
              << std::endl;
    std::cout << "y_sf (C8): " << y_sf_.getComponents(center.getCoordinateSystem())
              << std::endl;

    std::cout << "showerAxisDirection (conexObservationCS): "
              << showerAxis_.getDirection().getComponents(conexObservationCS_)
              << std::endl;
    std::cout << "showerAxisDirection (C8): "
              << showerAxis_.getDirection().getComponents(center.getCoordinateSystem())
              << std::endl;

    std::cout << "showerCore (conexObservationCS): "
              << showerCore_.getCoordinates(conexObservationCS_) << std::endl;
    std::cout << "showerCore (C8): "
              << showerCore_.getCoordinates(center.getCoordinateSystem()) << std::endl;

    int randomSeeds[3] = {1234, 0, 0}; // will be overwritten later??
    int heModel = eSibyll23;

    int nShower = 1; // large to avoid final stats.
    int maxDetail = 0;
#ifdef CONEX_EXTENSIONS
    int particleListMode = 0;
#endif

    std::string configPath = CONEX_CONFIG_PATH;
    ::conex::initconex_(nShower, randomSeeds, heModel, maxDetail,
#ifdef CONEX_EXTENSIONS
                        particleListMode,
#endif
                        configPath.c_str(), configPath.size());

    double eprima = primaryEnergy / 1_GeV;

    // set phi, theta
    Vector<length_d> ez{conexObservationCS_, {0._m, 0._m, -1_m}};
    auto const c = showerAxis_.getDirection().dot(ez) / 1_m;
    double theta = std::acos(c) * 180 / M_PI;

    auto const showerAxisConex =
        showerAxis_.getDirection().getComponents(conexObservationCS_);
    double phi = std::atan2(-showerAxisConex.getY().magnitude(),
                            showerAxisConex.getX().magnitude()) *
                 180 / M_PI;

    std::cout << "theta (deg) = " << theta << "; phi (deg) = " << phi << std::endl;

    int ipart = static_cast<int>(primaryPDG);
    auto rng = RNGManager::getInstance().getRandomStream("cascade");

    double dimpact = 0.; // valid only if shower core is fixed on the observation plane;
                         // for skimming showers an offset is needed like in CONEX

    std::array<int, 3> ioseed{static_cast<int>(rng()), static_cast<int>(rng()),
                              static_cast<int>(rng())};

    double xminp = injectionHeight / 1_m;

    ::conex::conexrun_(ipart, eprima, theta, phi, xminp, dimpact, ioseed.data());
  }

  void CONEXhybrid::doSecondaries(setup::StackView& vS) {
    auto p = vS.begin();
    while (p != vS.end()) {
      Code const pid = p.getPID();
      if (addParticle(pid, p.getEnergy(), p.getMass(), p.getPosition(),
                      p.getMomentum().normalized(), p.getTime())) {
        p.erase();
      }
      ++p;
    }
  }

  bool CONEXhybrid::addParticle(Code pid, HEPEnergyType energy, HEPEnergyType mass,
                                Point const& position, DirectionVector const& direction,
                                TimeType t) {

    auto const it = std::find_if(egs_em_codes_.cbegin(), egs_em_codes_.cend(),
                                 [=](auto const& p) { return pid == p.first; });
    if (it == egs_em_codes_.cend()) { return false; }

    // EM particle
    auto const egs_pid = it->second;
    std::cout << "position conexObs: " << position.getCoordinates(conexObservationCS_)
              << std::endl;

    auto const coords = position.getCoordinates(conexObservationCS_) / 1_m;
    double const x = coords[0].magnitude();
    double const y = coords[1].magnitude();

    double const altitude = ((position - center_).getNorm() - conex::earthRadius) / 1_m;
    auto const d = position - showerCore_;

    // distance from core to particle projected along shower axis
    double const slantDistance = -d.dot(showerAxis_.getDirection()) / 1_m;

    // lateral coordinates in CONEX shower frame
    auto const dShowerPlane = d - d.getParallelProjectionOnto(showerAxis_.getDirection());
    double const lateralX = dShowerPlane.dot(x_sf_) / 1_m;
    double const lateralY = dShowerPlane.dot(y_sf_) / 1_m;

    double const slantX = showerAxis_.getProjectedX(position) * (1_cm * 1_cm / 1_g);

    double const time = (t * constants::c - groundDist_) / 1_m;

    // fill u,v,w momentum direction in EGS frame
    double const u = direction.dot(y_sf_).magnitude();
    double const v = direction.dot(x_sf_).magnitude();
    double const w = direction.dot(showerAxis_.getDirection()).magnitude();

    double const weight = 1; // NEEDS TO BE CHANGED WHEN WE HAVE WEIGHTS!

    // generation, TO BE CHANGED WHEN WE HAVE THAT INFORMATION AVAILABLE
    int const latchin = 1;

    double const E = energy / 1_GeV;
    double const m = mass / 1_GeV;

    std::cout << "CONEXhybrid: removing " << egs_pid << " " << std::scientific << energy
              << " GeV" << std::endl;

    std::cout << "#### parameters to cegs4_() ####" << std::endl;
    std::cout << "egs_pid = " << egs_pid << std::endl;
    std::cout << "E = " << E << std::endl;
    std::cout << "m = " << m << std::endl;
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

    ::conex::cxoptl_.dptl[10 - 1] = egs_pid;
    ::conex::cxoptl_.dptl[4 - 1] = E;
    ::conex::cxoptl_.dptl[5 - 1] = m;
    ::conex::cxoptl_.dptl[6 - 1] = x;
    ::conex::cxoptl_.dptl[7 - 1] = y;
    ::conex::cxoptl_.dptl[8 - 1] = altitude;
    ::conex::cxoptl_.dptl[9 - 1] = time;
    ::conex::cxoptl_.dptl[11 - 1] = weight;
    ::conex::cxoptl_.dptl[12 - 1] = latchin;
    ::conex::cxoptl_.dptl[13 - 1] = slantX;
    ::conex::cxoptl_.dptl[14 - 1] = lateralX;
    ::conex::cxoptl_.dptl[15 - 1] = lateralY;
    ::conex::cxoptl_.dptl[16 - 1] = slantDistance;
    ::conex::cxoptl_.dptl[2 - 1] = u;
    ::conex::cxoptl_.dptl[1 - 1] = v;
    ::conex::cxoptl_.dptl[3 - 1] = w;

    int n = 1, i = 1;
    ::conex::cegs4_(n, i);

    return true;
  }

  void CONEXhybrid::solveCE() {

    ::conex::conexcascade_();

    int nX = ::conex::get_number_of_depth_bins_(); // make sure this works!

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

    float EGround[3], fitpars[13];

    ::conex::get_shower_data_(icut, iSec, nX, X[0], N[0], fitpars[0], H[0], D[0]);
    ::conex::get_shower_edep_(icut, nX, dEdX[0], EGround[0]);
    ::conex::get_shower_muon_(icutm, nX, Mu[0], dMu[0]);
    ::conex::get_shower_gamma_(icutg, nX, Gamma[0]);
    ::conex::get_shower_electron_(icute, nX, Electrons[0]);
    ::conex::get_shower_hadron_(icuth, nX, Hadrons[0]);

    std::ofstream file{"conex_output.txt"};
    file << fmt::format("#{:>8} {:>13} {:>13} {:>13} {:>13} {:>13} {:>13} {:>13}\n", "X",
                        "N", "dEdX", "Mu", "dMu", "Gamma", "El", "Had");
    for (int i = 0; i < nX; ++i) {
      file << fmt::format(
          " {:>8.2f} {:>13.3} {:>13.3} {:>13.3} {:>13.3} {:>13.3} {:>13.3} {:>13.3}\n",
          X[i], N[i], dEdX[i], Mu[i], dMu[i], Gamma[i], Electrons[i], Hadrons[i]);
    }
    file.close();

    std::ofstream fitout{"conex_fit.txt"};
    fitout << fitpars[1 - 1] << " # log10(eprima/eV)" << std::endl;
    fitout << fitpars[2 - 1] << " # theta" << std::endl;
    fitout << fitpars[3 - 1] << " # X1 (first interaction)" << std::endl;
    fitout << fitpars[4 - 1] << " # Nmax" << std::endl;
    fitout << fitpars[5 - 1] << " # X0" << std::endl;
    fitout << fitpars[6 - 1] << " # P1" << std::endl;
    fitout << fitpars[7 - 1] << " # P2" << std::endl;
    fitout << fitpars[8 - 1] << " # P3" << std::endl;
    fitout << fitpars[9 - 1] << " # chi^2 / sqrt(Nmax)" << std::endl;
    fitout << fitpars[10 - 1] << " # Xmax" << std::endl;
    fitout << fitpars[11 - 1] << " # phi" << std::endl;
    fitout << fitpars[12 - 1] << " # inelasticity 1st int." << std::endl;
    fitout << fitpars[13 - 1] << " # ???" << std::endl;
    fitout.close();
  }

} // namespace corsika