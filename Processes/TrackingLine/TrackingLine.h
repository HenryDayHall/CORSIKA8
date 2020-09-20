/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/quartic.h>
#include <optional>
#include <type_traits>
#include <utility>

#include <fstream>
#include <iostream>
#include <boost/histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <corsika/process/tracking_line/dump_bh.hpp>

namespace corsika::environment {
  template <typename IEnvironmentModel>
  class Environment;
  template <typename IEnvironmentModel>
  class VolumeTreeNode;
} // namespace corsika::environment

using namespace boost::histogram;
/*static auto histL = make_histogram(axis::regular<>(100, 0, 100000, "Leap-Frog-ength L'"));
static auto histS = make_histogram(axis::regular<>(100, 0, 100000, "Direct Length S"));
static auto histB = make_histogram(axis::regular<>(100, 0, 100000, "Arc Length B")); */
static auto histLlog = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "Leap-Frog-ength L'"));
static auto histLloggeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "Leap-Frog-length L for geometric steps"));
static auto histLlogmag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "Leap-Frog-length L for magnetic steps"));
static auto histSlog = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "Direct Length S"));
static auto histBlog = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "Arc Length B"));
/*static auto histLB = make_histogram(axis::regular<>(100, 0, 100, "L - B"));
static auto histLS = make_histogram(axis::regular<>(100, 0, 100, "L - S"));*/
static auto histLBrelgeo = make_histogram(axis::regular<double, axis::transform::log> (40,1e-12,1e-2,"L/B -1"));
static auto histLBrelmag = make_histogram(axis::regular<double, axis::transform::log> (40,1e-12,1e-2,"L/B -1"));
static auto histLSrelgeo = make_histogram(axis::regular<double, axis::transform::log>(40,1e-12,1e-2, "L/S -1"));
static auto histLSrelmag = make_histogram(axis::regular<double, axis::transform::log>(40,1e-12,1e-2, "L/S -1"));
static auto histELSrel = make_histogram(axis::regular<double, axis::transform::log>(50,1e-8,1e-3, "L/S -1"),axis::regular<double, axis::transform::log>(30, 3, 3e1, "E / GeV"));
static auto histELSrelp = make_histogram(axis::regular<double, axis::transform::log>(50,1e-8,1e-3, "L/S -1"),axis::regular<double, axis::transform::log>(30, 3, 3e1, "E / GeV"));
static auto histELSrelpi = make_histogram(axis::regular<double, axis::transform::log>(50,1e-8,1e-3, "L/S -1"),axis::regular<double, axis::transform::log>(30, 3, 3e1, "E / GeV"));
static auto histELSrelmu = make_histogram(axis::regular<double, axis::transform::log>(50,1e-8,1e-3, "L/S -1"),axis::regular<double, axis::transform::log>(30, 3, 3e1, "E / GeV"));
static auto histELSrele = make_histogram(axis::regular<double, axis::transform::log>(50,1e-8,1e-3, "L/S -1"),axis::regular<double, axis::transform::log>(30, 3, 3e1, "E / GeV"));
//static auto histBS = make_histogram(axis::regular<>(100, 0, 0.01, "B - S"));
static auto histLpgeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Protonen"));
static auto histLpigeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Pionen"));
static auto histLmugeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Myonen"));
static auto histLegeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Elektronen"));
static auto histLygeo = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Photonen"));
static auto histLpmag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Protonen"));
static auto histLpimag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Pionen"));
static auto histLmumag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Myonen"));
static auto histLemag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Elektronen"));
static auto histLymag = make_histogram(axis::regular<double, axis::transform::log>(100, 1e-3, 1e7, "L' für Photonen"));
static double L = 0;
static std::vector<double> Lsmall;
static std::vector<double> Bbig;
static std::vector<double> Sbig;
static std::vector<double> Lsmallmag;
static std::vector<double> Bbigmag;
static std::vector<double> Sbigmag;
static std::vector<double> Emag;
static std::vector<double> E;
static std::vector<double> Steplimitmag;


namespace corsika::process {

  namespace tracking_line {
  
    std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
    TimeOfIntersection(geometry::Line const&, geometry::Sphere const&);

    corsika::units::si::TimeType TimeOfIntersection(geometry::Line const&,
                                                    geometry::Plane const&);
                                                  
    class TrackingLine {

    public:
      TrackingLine() = default;
      ~TrackingLine(){
		  /*std::ofstream myfile;
          myfile.open ("histograms.txt");
          myfile << histLlog << std::endl;
          myfile << histBlog << std::endl;
          myfile << histLB << std::endl;
          myfile << histLS << std::endl;
          myfile.close(); 
          
          std::ofstream myfile2;
          myfile2.open ("lengen.txt");
          myfile2 << "L " << std::endl;
          for(double n : Lsmall) {
            myfile2 << n << '\n';
          }
          myfile2 << "B " << std::endl;
          for(double n : Bbig) {
            myfile2 << n << '\n';
          }
          myfile2 << "S " << std::endl;
          for(double n : Sbig) {
            myfile2 << n << '\n';
          }
          myfile2 << "E in GeV" << std::endl;
          for(double n : E) {
            myfile2 << n << '\n';
          }
          myfile2 << "L mag" << std::endl;
          for(double n : Lsmallmag) {
            myfile2 << n << '\n';
          }
          myfile2 << "B mag" << std::endl;
          for(double n : Bbigmag) {
            myfile2 << n << '\n';
          }
          myfile2 << "S mag" << std::endl;
          for(double n : Sbigmag) {
            myfile2 << n << '\n';
          }
          myfile2 << "E mag in GeV" << std::endl;
          for(double n : Emag) {
            myfile2 << n << '\n';
          }
          myfile2 << "Schrittlänge" << std::endl;
          for(double n : Steplimitmag) {
            myfile2 << n << '\n';
          }
          myfile2.close();
		  
          std::ofstream file1("histL.json");
          dump_bh(file1, histL);
          file1.close();
          std::ofstream file2("histS.json");
          dump_bh(file2, histS);
          file2.close();
          std::ofstream file3("histB.json");
          dump_bh(file3, histB);
          file3.close();
          std::ofstream file4("histLB.json");
          dump_bh(file4, histLB);
          file4.close();
          std::ofstream file5("histLS.json");
          dump_bh(file5, histLS);
          file5.close();
          std::ofstream file6("histBS.json");
          dump_bh(file6, histBS);
          file6.close();*/
          std::ofstream file7("histLBrelgeo.json");
          dump_bh(file7, histLBrelgeo);
          file7.close();
          std::ofstream file8("histLSrelgeo.json");
          dump_bh(file8, histLSrelgeo);
          file8.close();
          std::ofstream file9("histLBrelmag.json");
          dump_bh(file9, histLBrelmag);
          file9.close();
          std::ofstream file0("histLSrelmag.json");
          dump_bh(file0, histLSrelmag);
          file0.close();
          
          std::ofstream file10("histELSrel.json");
          dump_bh(file10, histELSrel);
          file10.close();
          std::ofstream file11("histLmugeo.json");
          dump_bh(file11, histLmugeo);
          file11.close();
          std::ofstream file12("histLpigeo.json");
          dump_bh(file12, histLpigeo);
          file12.close();
          std::ofstream file13("histLpgeo.json");
          dump_bh(file13, histLpgeo);
          file13.close();
          std::ofstream file14("histLlog.json");
          dump_bh(file14, histLlog);
          file14.close();
          std::ofstream file15("histBlog.json");
          dump_bh(file15, histBlog);
          file15.close();
          std::ofstream file16("histSlog.json");
          dump_bh(file16, histSlog);
          file16.close();
          std::ofstream file17("histELSrelmu.json");
          dump_bh(file17, histELSrelmu);
          file17.close();
          std::ofstream file18("histELSrelp.json");
          dump_bh(file18, histELSrelp);
          file18.close();
          std::ofstream file19("histELSrelpi.json");
          dump_bh(file19, histELSrelpi);
          file19.close();
          
          std::ofstream file21("histLgeo.json");
          dump_bh(file21, histLloggeo);
          file21.close();
          std::ofstream file22("histLmag.json");
          dump_bh(file22, histLlogmag);
          file22.close();
          std::ofstream file23("histLmumag.json");
          dump_bh(file23, histLmumag);
          file23.close();
          std::ofstream file24("histLpimag.json");
          dump_bh(file24, histLpimag);
          file24.close();
          std::ofstream file25("histLpmag.json");
          dump_bh(file25, histLpmag);
          file25.close();
          std::ofstream file26("histLemag.json");
          dump_bh(file26, histLemag);
          file26.close();
          std::ofstream file27("histLegeo.json");
          dump_bh(file27, histLegeo);
          file27.close();
          std::ofstream file28("histELSrele.json");
          dump_bh(file28, histELSrele);
          file28.close();
          std::ofstream file29("histLymag.json");
          dump_bh(file29, histLymag);
          file29.close();
          std::ofstream file20("histLygeo.json");
          dump_bh(file20, histLygeo);
          file20.close();
		  
		  };

      template <typename Particle> // was Stack previously, and argument was
                                   // Stack::StackIterator
      auto GetTrack(Particle const& p) {
        using namespace corsika::units::si;
        using namespace corsika::geometry;
        geometry::Vector<SpeedType::dimension_type> velocity =
            p.GetMomentum() / p.GetEnergy() * corsika::units::constants::c;

        auto const currentPosition = p.GetPosition();
        std::cout << "TrackingLine pid: " << p.GetPID()
                  << " , E = " << p.GetEnergy() / 1_GeV << " GeV" << std::endl;
        std::cout << "TrackingLine pos: "
                  << currentPosition.GetCoordinates()
                  // << " [" << p.GetNode()->GetModelProperties().GetName() << "]"
                  << std::endl;
        std::cout << "TrackingLine   E: " << p.GetEnergy() / 1_GeV << " GeV" << std::endl;
        std::cout << "TrackingLine   p: " << p.GetMomentum().GetComponents() / 1_GeV
                  << " GeV " << std::endl;
        std::cout << "TrackingLine   v: " << velocity.GetComponents() << std::endl;
        
        geometry::Line line(currentPosition, velocity);
        
        auto const* currentLogicalVolumeNode = p.GetNode();
        //~ auto const* currentNumericalVolumeNode =
        //~ fEnvironment.GetUniverse()->GetContainingNode(currentPosition);
        auto const numericallyInside =
            currentLogicalVolumeNode->GetVolume().Contains(currentPosition);

        std::cout << "numericallyInside = " << (numericallyInside ? "true" : "false") << std::endl;

        auto const& children = currentLogicalVolumeNode->GetChildNodes();
        auto const& excluded = currentLogicalVolumeNode->GetExcludedNodes();

        std::vector<std::pair<TimeType, decltype(p.GetNode())>> intersections;
        
        //charge of the particle
        int chargeNumber = 0;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
   	    std::cout << " Magnetic Field: " << magneticfield.GetComponents() / 1_uT << " uT " << std::endl;
        LengthType Steplength1 = 0_m;
        LengthType Steplength2 = 0_m;
        LengthType Steplimit = 1_m / 0;
        //infinite kann man auch anders schreiben
        bool intersection = true;
        
        // for entering from outside
        auto addIfIntersects = [&](auto const& vtn) {
          auto const& volume = vtn.GetVolume();
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              volume); // for the moment we are a bit bold here and assume
          // everything is a sphere, crashes with exception if not
          
          // creating Line with magnetic field
          if (chargeNumber != 0) {
            auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / (velocity.norm() * p.GetEnergy() * 1_V);
            geometry::Vector<dimensionless_d> const directionBefore = velocity.normalized();
          	// determine steplength to next volume
            double a = ((directionBefore.cross(magneticfield)).dot(currentPosition - sphere.GetCenter()) * k + 1) * 4 / 
                      (1_m * 1_m * (directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k);
            double b = directionBefore.dot(currentPosition - sphere.GetCenter()) * 8 / 
                      ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k * 1_m * 1_m * 1_m);
            double c = ((currentPosition - sphere.GetCenter()).GetSquaredNorm() - 
                      (sphere.GetRadius() * sphere.GetRadius())) * 4 /
                      ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k * 1_m * 1_m * 1_m * 1_m);
            std::complex<double>*  solutions = solve_quartic(0, a, b, c);
            std::vector<double> tmp;
            for (int i = 0; i < 4; i++) {
              if (solutions[i].imag() == 0 && solutions[i].real() > 0) {
                tmp.push_back(solutions[i].real());
                std::cout << "Solutions for next Volume: " << solutions[i].real() << std::endl;
              }
            }
            if (tmp.size() > 0) {
              Steplength1 = 1_m * *std::min_element(tmp.begin(),tmp.end());
              std::cout << "Steplength to next volume = " << Steplength1 << std::endl;
              std::cout << "Time to next volume = " << Steplength1 / velocity.norm() << std::endl;
            } else {
              std::cout << "no intersection (1)!" << std::endl;
              intersection = false;
            }
            delete [] solutions;
          }
          
          if (intersection) {
            auto line1 = MagneticStep(p, line, Steplength1, false);
            // instead of changing the line with magnetic field, the TimeOfIntersection() could be changed
            // using line has some errors for huge steps
  
            if (auto opt = TimeOfIntersection(line1, sphere); opt.has_value()) {
              auto const [t1, t2] = *opt;
              std::cout << "intersection times: " << t1 / 1_s << "; "
                        << t2 / 1_s
                        // << " " << vtn.GetModelProperties().GetName()
                        << std::endl;
              if (t1.magnitude() > 0)
                intersections.emplace_back(t1, &vtn);
              else if (t2.magnitude() > 0)
                std::cout << "inside other volume" << std::endl;
            }
          }
        };

        for (auto const& child : children) { addIfIntersects(*child); }
        for (auto const* ex : excluded) { addIfIntersects(*ex); }

        {
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              currentLogicalVolumeNode->GetVolume());
          // for the moment we are a bit bold here and assume
          // everything is a sphere, crashes with exception if not
          
          // creating Line with magnetic field
          if (chargeNumber != 0) {
            auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / (velocity.norm() * p.GetEnergy() * 1_V);
            geometry::Vector<dimensionless_d> const directionBefore = velocity.normalized();
          	// determine steplength to next volume
            double a = ((directionBefore.cross(magneticfield)).dot(currentPosition - sphere.GetCenter()) * k + 1) * 4 / 
                      (1_m * 1_m * (directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k);
            double b = directionBefore.dot(currentPosition - sphere.GetCenter()) * 8 / 
                      ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k * 1_m * 1_m * 1_m);
            double c = ((currentPosition - sphere.GetCenter()).GetSquaredNorm() - 
                      (sphere.GetRadius() * sphere.GetRadius())) * 4 /
                      ((directionBefore.cross(magneticfield)).GetSquaredNorm() * k * k * 1_m * 1_m * 1_m * 1_m);
            std::complex<double>*  solutions = solve_quartic(0, a, b, c);
            std::vector<double> tmp;
            for (int i = 0; i < 4; i++) {
              if (solutions[i].imag() == 0 && solutions[i].real() > 0) {
                tmp.push_back(solutions[i].real());
                std::cout << "Solutions for current Volume: " << solutions[i].real() << std::endl;
              }
            }
            if (tmp.size() > 0) {
			  Steplength2 = 1_m * *std::min_element(tmp.begin(),tmp.end());
			  if (numericallyInside == false) {
			    int p = std::min_element(tmp.begin(),tmp.end()) - tmp.begin();
				tmp.erase(tmp.begin() + p);
				Steplength2 = 1_m * *std::min_element(tmp.begin(),tmp.end());
			  }
			  std::cout << "steplength out of current volume = " << Steplength2 << std::endl;
              std::cout << "Time out of current volume = " << Steplength2 / velocity.norm() << std::endl;
            } else {
              std::cout << "no intersection (2)!" << std::endl;
              // what to do when this happens? (very unlikely)
            }
            delete [] solutions;
            
            geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
            velocity.parallelProjectionOnto(magneticfield);
            LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
					                            (corsika::units::constants::cSquared * abs(chargeNumber) * 
					                            magneticfield.GetNorm() * 1_eV);
            Steplimit = 2 * sin(0.1) * gyroradius;
            std::cout << "Steplimit: " << Steplimit << std::endl;
          }
		
          auto line2 = MagneticStep(p, line, Steplength2, false);
          // instead of changing the line with magnetic field, the TimeOfIntersection() could be changed
          // using line has some errors for huge steps
          
          [[maybe_unused]] auto const [t1, t2] = *TimeOfIntersection(line2, sphere);
          [[maybe_unused]] auto dummy_t1 = t1;
          intersections.emplace_back(t2, currentLogicalVolumeNode->GetParent());
        }

        auto const minIter = std::min_element(
            intersections.cbegin(), intersections.cend(),
            [](auto const& a, auto const& b) { return a.first < b.first; });

        TimeType min;

        if (minIter == intersections.cend()) {
          min = 1_s; // todo: do sth. more reasonable as soon as tracking is able
          // to handle the numerics properly
          throw std::runtime_error("no intersection with anything!");
        } else {
          min = minIter->first;
        }

        std::cout << " t-intersect: "
                  << min
                  // << " " << minIter->second->GetModelProperties().GetName()
                  << std::endl;
        
        //if the used Steplengths are too big, min gets false and we do another Step
        std::cout << Steplength1 << Steplength2 << std::endl;
        std::cout << intersection << " " << Steplimit << std::endl;
        if ((Steplength1 > Steplimit || Steplength1 == 0_m) && Steplength2 > Steplimit) {
          min = Steplimit / velocity.norm();
          auto lineWithB = MagneticStep(p, line, Steplimit, true);
          
 			      //histL(L);
            histLlog(L);
            histLlogmag(L);
            //histS(lineWithB.ArcLength(0_s,min) / 1_m);
            histSlog(lineWithB.ArcLength(0_s,min) / 1_m);
            //histLS(L-lineWithB.ArcLength(0_s,min) / 1_m);
                        
            if(chargeNumber != 0) {
              if(L * 1_m/lineWithB.ArcLength(0_s,min) -1 > 0) {
                histLSrelmag(L * 1_m/lineWithB.ArcLength(0_s,min) -1);
                //histELSrel(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
              } else {
Lsmallmag.push_back(L); Sbigmag.push_back(lineWithB.ArcLength(0_s,min) / 1_m);
Emag.push_back(p.GetEnergy() / 1_GeV);Steplimitmag.push_back(Steplimit / 1_m);
              }
              auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
              geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
                   velocity.parallelProjectionOnto(magneticfield);
              LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
                                            (corsika::units::constants::cSquared * abs(chargeNumber) * 
                                            magneticfield.GetNorm() * 1_eV);
              std::cout << "Schrittlaenge " << velocity.norm() * min << " gyroradius " << gyroradius << std::endl;
              LengthType B = 2 * gyroradius * asin ( lineWithB.ArcLength(0_s,min) / 2 / gyroradius);
              std::cout << "Bogenlaenge" << B << std::endl;
              //histB(B / 1_m);
              histBlog(B / 1_m);
              if(L-B/1_m > 0) {
                //histLB(L-B/1_m);
                //histBS(B/1_m - lineWithB.ArcLength(0_s,min) / 1_m);
                histLBrelmag(L * 1_m/B-1);
              } else {
Bbigmag.push_back(B / 1_m);
              }
            } else {
            //histB(lineWithB.ArcLength(0_s,min) / 1_m);
            histBlog(lineWithB.ArcLength(0_s,min) / 1_m);
            }
            
            int pdg = static_cast<int>(particles::GetPDG(p.GetPID()));
            if (abs(pdg) == 13)
              histLmumag(L);
            if (abs(pdg) == 211 || abs(pdg) == 111)
              histLpimag(L);
            if (abs(pdg) == 2212 || abs(pdg) == 2112)
              histLpmag(L);
            if (abs(pdg) == 11)
              histLemag(L);
            if (abs(pdg) == 22)
              histLymag(L);
          
          return std::make_tuple(geometry::Trajectory<geometry::Line>(line, min),
                               geometry::Trajectory<geometry::Line>(lineWithB, min),
                               Steplimit * 1.0001, Steplimit, minIter->second);
        }
        
        auto lineWithB = MagneticStep(p, line, velocity.norm() * min, true);
        
			      //histL(L);
            histLlog(L);
            histLloggeo(L);
            //histS(lineWithB.ArcLength(0_s,min) / 1_m);
            histSlog(lineWithB.ArcLength(0_s,min) / 1_m);
            //histLS(L-lineWithB.ArcLength(0_s,min) / 1_m);
                        
            if(chargeNumber != 0) {
              if(L * 1_m/lineWithB.ArcLength(0_s,min) -1 > 0) {
                histLSrelgeo(L * 1_m/lineWithB.ArcLength(0_s,min) -1);
                histELSrel(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
              } else {
Lsmall.push_back(L); Sbig.push_back(lineWithB.ArcLength(0_s,min) / 1_m);
E.push_back(p.GetEnergy() / 1_GeV);
              }
              auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
              geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
                   velocity.parallelProjectionOnto(magneticfield);
              LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
                                            (corsika::units::constants::cSquared * abs(chargeNumber) * 
                                            magneticfield.GetNorm() * 1_eV);
              std::cout << "Schrittlaenge " << velocity.norm() * min << " gyroradius " << gyroradius << std::endl;
              LengthType B = 2 * gyroradius * asin ( lineWithB.ArcLength(0_s,min) / 2 / gyroradius);
              std::cout << "Bogenlaenge" << B << std::endl;
              //histB(B / 1_m);
              histBlog(B / 1_m);
              if(L-B/1_m > 0) {
                //histLB(L-B/1_m);
                //histBS(B/1_m - lineWithB.ArcLength(0_s,min) / 1_m);
                histLBrelgeo(L * 1_m/B-1);
              } else {
Bbig.push_back(B / 1_m);
              }
            } else {
            //histB(lineWithB.ArcLength(0_s,min) / 1_m);
            histBlog(lineWithB.ArcLength(0_s,min) / 1_m);
            }
            
            int pdg = static_cast<int>(particles::GetPDG(p.GetPID()));
            if (abs(pdg) == 13)  {
              histLmugeo(L);
              histELSrelmu(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
            }
            if (abs(pdg) == 211 || abs(pdg) == 111) {
              histLpigeo(L);
              if (chargeNumber != 0) 
                histELSrelpi(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
            }
            if (abs(pdg) == 2212 || abs(pdg) == 2112){
              histLpgeo(L);
              if (chargeNumber != 0) 
                histELSrelp(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
            }
            if (abs(pdg) == 11){
              histLegeo(L);
              if (chargeNumber != 0) 
                histELSrele(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
            }
            if (abs(pdg) == 22)
              histLymag(L);

        
        return std::make_tuple(geometry::Trajectory<geometry::Line>(line, min),
                               geometry::Trajectory<geometry::Line>(lineWithB, min),
                               velocity.norm() * min, Steplimit, minIter->second);
      }
      
      template <typename Particle> // was Stack previously, and argument was
                                   // Stack::StackIterator
                                   
      // determine direction of the particle after adding magnetic field
      corsika::geometry::Line MagneticStep(
      Particle const& p, corsika::geometry::Line line, corsika::units::si::LengthType Steplength, bool i) {
        using namespace corsika::units::si;
      
        if (line.GetV0().norm() * 1_s == 0_m) 
          return line;
        
        //charge of the particle
        int chargeNumber;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        
        auto const* currentLogicalVolumeNode = p.GetNode();
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(p.GetPosition());
        auto k = chargeNumber * corsika::units::constants::c * 1_eV / (p.GetMomentum().norm() * 1_V);
        geometry::Vector<dimensionless_d> direction = line.GetV0().normalized();
        auto position = p.GetPosition();
          
        // First Movement
        // assuming magnetic field does not change during movement
        position = position + direction * Steplength / 2;
        // Change of direction by magnetic field
        direction = direction + direction.cross(magneticfield) * Steplength * k;
        // Second Movement
        position = position + direction * Steplength / 2;
        
        if(i == true) {
		      //if(chargeNumber != 0) {
              L = direction.norm() * Steplength / 2_m + Steplength / 2_m;
          //}
        }
          
        geometry::Vector<LengthType::dimension_type> const distance = position - p.GetPosition();
        if(distance.norm() == 0_m) {
          return line;
        } 
        geometry::Line newLine(p.GetPosition(), distance.normalized() * line.GetV0().norm());
        return newLine;
      }
      
      template <typename Particle> // was Stack previously, and argument was
                                   // Stack::StackIterator
                                   
      // determine direction of the particle after adding magnetic field
      auto MagneticStep(Particle const& p, corsika::units::si::LengthType Steplength) {
        using namespace corsika::units::si;
        
        if (p.GetMomentum().norm() == 0_GeV) {
          return std::make_tuple(p.GetPosition(), p.GetMomentum() / 1_GeV, double(0));
        }
        
        //charge of the particle
        int chargeNumber;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        
        auto const* currentLogicalVolumeNode = p.GetNode();
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(p.GetPosition());
        auto k = chargeNumber * corsika::units::constants::c * 1_eV / (p.GetMomentum().norm() * 1_V);
        geometry::Vector<dimensionless_d> direction = p.GetMomentum().normalized();
        auto position = p.GetPosition();

        // First Movement
        // assuming magnetic field does not change during movement
        position = position + direction * Steplength / 2;
        // Change of direction by magnetic field
        direction = direction + direction.cross(magneticfield) * Steplength * k;
        // Second Movement
        position = position + direction * Steplength / 2;
        auto L2 = direction.norm() * Steplength / 2_m + Steplength / 2_m;
        return std::make_tuple(position, direction.normalized(), L2);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process
