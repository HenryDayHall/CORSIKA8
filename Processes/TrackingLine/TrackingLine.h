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
static auto histL = make_histogram(axis::regular<>(100, 0, 60000, "L'"));
static auto histS = make_histogram(axis::regular<>(100, 0, 60000, "S"));
static auto histB = make_histogram(axis::regular<>(100, 0, 60000, "Bogenlänge"));
static auto histLB = make_histogram(axis::regular<>(100, 0, 0.01, "L - B"));
static auto histLS = make_histogram(axis::regular<>(100, 0, 0.01, "L - S"));
static auto histLBrel = make_histogram(axis::regular<double, axis::transform::log> (20,1e-11,1e-6,"L/B -1"));
static auto histLSrel = make_histogram(axis::regular<double, axis::transform::log>(20,1e-11,1e-6, "L/S -1"));
static auto histELSrel = make_histogram(axis::regular<double, axis::transform::log>(20,1e-11,1e-6, "L/S -1"),axis::regular<double, axis::transform::log>(20, 0.1, 1e4, "E / GeV"));
static auto histBS = make_histogram(axis::regular<>(100, 0, 0.01, "B - S"));
static auto histLp = make_histogram(axis::regular<>(100, 0, 60000, "L' für Protonen"));
static auto histLpi = make_histogram(axis::regular<>(100, 0, 60000, "L' für Pionen"));
static auto histLmu = make_histogram(axis::regular<>(100, 0, 60000, "L' für Myonen"));
//static auto histLe = make_histogram(axis::regular<>(100, 0, 60000, "L' für Elektronen"));
//static auto histLy = make_histogram(axis::regular<>(100, 0, 60000, "L' für Photonen"));
static double L = 0;

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
		  std::ofstream myfile;
          myfile.open ("histograms.txt");
          myfile << histLB << std::endl;
          myfile << histLBrel << std::endl;
          myfile << histLS << std::endl;
          myfile << histLSrel << std::endl;
          myfile << histELSrel << std::endl;
          myfile.close(); 
		  
		  /*std::cout << histLBrel << std::endl;
		  std::cout << histLSrel << std::endl;*/

		  
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
          file6.close();
          std::ofstream file7("histLBrel.json");
          dump_bh(file7, histLBrel);
          file7.close();
          std::ofstream file8("histLSrel.json");
          dump_bh(file8, histLSrel);
          file8.close();
          
          std::ofstream file10("histELSrel.json");
          dump_bh(file10, histELSrel);
          file10.close();
          std::ofstream file11("histLmu.json");
          dump_bh(file11, histLmu);
          file11.close();
          std::ofstream file12("histLpi.json");
          dump_bh(file12, histLpi);
          file12.close();
          std::ofstream file13("histLp.json");
          dump_bh(file13, histLp);
          file13.close();
		  
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
        int chargeNumber;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
   	    std::cout << " Magnetic Field: " << magneticfield.GetComponents() / 1_uT << " uT " << std::endl;
        auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / (velocity.GetNorm() * p.GetEnergy() * 1_V);
        geometry::Vector<dimensionless_d> const directionBefore = velocity.normalized();
        LengthType Steplength1 = 10_m; // length irrelevant if q=0 and else it gets changed again
        LengthType Steplength2 = 10_m;
        LengthType Steplimit = 1_m / 0;
        //infinite kann man auch anders schreiben          

        // for entering from outside
        auto addIfIntersects = [&](auto const& vtn) {
          auto const& volume = vtn.GetVolume();
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              volume); // for the moment we are a bit bold here and assume
          // everything is a sphere, crashes with exception if not
          
          // creating Line with magnetic field
          if (chargeNumber != 0) {        
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
              // what to do when this happens? (very unlikely)
            }
            delete [] solutions;
            
            geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
            velocity.parallelProjectionOnto(magneticfield);
            LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
					                            (corsika::units::constants::cSquared * abs(chargeNumber) * 
					                            magneticfield.GetNorm() * 1_eV);
            Steplimit = 2 * sin(0.1) * gyroradius;
          }
            
          auto line1 = MagneticStep(p, line, Steplength1, false);
          // instead of changing the line with magnetic field, the TimeOfIntersection() could be changed
          // using line has some errors for huge steps

          if (auto opt = TimeOfIntersection(line1, sphere); opt.has_value()) {
            auto const [t1, t2] = *opt;
            C8LOG_DEBUG("intersection times: {} s; {} s", t1 / 1_s, t2 / 1_s);
            if (t1.magnitude() > 0)
              intersections.emplace_back(t1, &vtn);
            else if (t2.magnitude() > 0)
              C8LOG_DEBUG("inside other volume");
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
                 
        auto lineWithB = MagneticStep(p, line, velocity.norm() * min, true);
        
        if(min * velocity.norm() < 1000000_m) {
			      histL(L);
            histS(lineWithB.ArcLength(0_s,min) / 1_m);
            histLS(L-lineWithB.ArcLength(0_s,min) / 1_m);
                        
            if(chargeNumber != 0) {
            histLSrel(L * 1_m/lineWithB.ArcLength(0_s,min) -1);
            histELSrel(L * 1_m/lineWithB.ArcLength(0_s,min) -1, p.GetEnergy() / 1_GeV);
            auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
            geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
                 velocity.parallelProjectionOnto(magneticfield);
            
            
              LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
                                            (corsika::units::constants::cSquared * abs(chargeNumber) * 
                                            magneticfield.GetNorm() * 1_eV);;
              LengthType B = 2 * gyroradius * asin ( lineWithB.ArcLength(0_s,min) / 2 / gyroradius);
              histB(B / 1_m);
              histLB(L-B/1_m);
              histBS(B/1_m - lineWithB.ArcLength(0_s,min) / 1_m);
              histLBrel(L * 1_m/B-1);
            }
            
            int pdg = static_cast<int>(particles::GetPDG(p.GetPID()));
            if (abs(pdg) == 13)
              histLmu(L);
            if (abs(pdg) == 211 || abs(pdg) == 111)
              histLpi(L);
            if (abs(pdg) == 2212 || abs(pdg) == 2112)
              histLp(L);
	        }

        
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
      
        //charge of the particle
        int chargeNumber;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        
        auto const* currentLogicalVolumeNode = p.GetNode();
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(p.GetPosition());
        auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / (line.GetV0().norm() * p.GetEnergy() * 1_V);
        geometry::Vector<dimensionless_d> direction = line.GetV0().normalized();
        auto position = p.GetPosition();
          
        // First Movement
        // assuming magnetic field does not change during movement
        position = position + direction * Steplength / 2;
        // Change of direction by magnetic field
        direction = direction + direction.cross(magneticfield) * Steplength * k;
        // Second Movement
        position = position + direction * Steplength / 2;
        
        if(i) {
		      //if(chargeNumber != 0) {
		        if(Steplength < 1000000_m) {
              L = direction.norm() * Steplength / 2_m + Steplength / 2_m;
            }
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
        
        //charge of the particle
        int chargeNumber;
        if (corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        
        auto const* currentLogicalVolumeNode = p.GetNode();
        auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(p.GetPosition());
        geometry::Vector<SpeedType::dimension_type> velocity =
                  p.GetMomentum() / p.GetEnergy() * corsika::units::constants::c;
        auto k = chargeNumber * corsika::units::constants::cSquared * 1_eV / (velocity.norm() * p.GetEnergy() * 1_V);
        geometry::Vector<dimensionless_d> direction = velocity.normalized();
        auto position = p.GetPosition();

        // First Movement
        // assuming magnetic field does not change during movement
        position = position + direction * Steplength / 2;
        // Change of direction by magnetic field
        direction = direction + direction.cross(magneticfield) * Steplength * k;
        // Second Movement
        position = position + direction * Steplength / 2;
        return std::make_tuple(position, direction.normalized());
        
      }
    };

  } // namespace tracking_line

} // namespace corsika::process
