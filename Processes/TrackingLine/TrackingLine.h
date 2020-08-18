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

namespace corsika::environment {
  template <typename IEnvironmentModel>
  class Environment;
  template <typename IEnvironmentModel>
  class VolumeTreeNode;
} // namespace corsika::environment

namespace corsika::process {

  namespace tracking_line {

    std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
    TimeOfIntersection(geometry::Line const&, geometry::Sphere const&);

    corsika::units::si::TimeType TimeOfIntersection(geometry::Line const&,
                                                    geometry::Plane const&);

    class TrackingLine {

    public:
      TrackingLine() = default;

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
        geometry::Vector<SpeedType::dimension_type> velocity1 = velocity;
        geometry::Vector<SpeedType::dimension_type> velocity2 = velocity;

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
            LengthType Steplength;
            if (tmp.size() > 0) {
              Steplength = 1_m * *std::min_element(tmp.begin(),tmp.end());
              std::cout << "Steplength to next volume = " << Steplength << std::endl;
            } else {
              std::cout << "no intersection (1)!" << std::endl;
              // what to do when this happens? (very unlikely)
            }
            delete [] solutions;
		
  		      // First Movement
  		      // assuming magnetic field does not change during movement
  		      auto position = currentPosition + directionBefore * Steplength / 2;
  		      // Change of direction by magnetic field
  		      geometry::Vector<dimensionless_d> const directionAfter = directionBefore + directionBefore.cross(magneticfield) *
                                                                    Steplength * k;
  		      // Second Movement
  		      position = position + directionAfter * Steplength / 2;
  		      geometry::Vector<dimensionless_d> const direction = (position - currentPosition) / 
  									                                            (position - currentPosition).GetNorm();
  		      velocity1 = direction * velocity.GetNorm();
          } // instead of changing the line with magnetic field, the TimeOfIntersection() could be changed
          // using line has some errors for huge steps
          geometry::Line line(currentPosition, velocity1);

          if (auto opt = TimeOfIntersection(line, sphere); opt.has_value()) {
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
            LengthType Steplength;
            if (tmp.size() > 0) {
				Steplength = 1_m * *std::min_element(tmp.begin(),tmp.end());
				if (numericallyInside == false) {
					int p = std::min_element(tmp.begin(),tmp.end()) - tmp.begin();
					tmp.erase(tmp.begin() + p);
					Steplength = 1_m * *std::min_element(tmp.begin(),tmp.end());
				}
				std::cout << "steplength out of current volume = " << Steplength << std::endl;
            } else {
              std::cout << "no intersection (2)!" << std::endl;
              // what to do when this happens? (very unlikely)
            }
            delete [] solutions;
		
  		      // First Movement
  		      // assuming magnetic field does not change during movement
  		      auto position = currentPosition + directionBefore * Steplength / 2;
  		      // Change of direction by magnetic field
  		      geometry::Vector<dimensionless_d> const directionAfter = directionBefore + directionBefore.cross(magneticfield) *
                                                                    Steplength * k;
  		      // Second Movement
  		      position = position + directionAfter * Steplength / 2;
  		      geometry::Vector<dimensionless_d> const direction = (position - currentPosition) / 
  									                                            (position - currentPosition).GetNorm();
  		      velocity2 = direction * velocity.GetNorm();
          } // instead of changing the line with magnetic field, the TimeOfIntersection() could be changed
          geometry::Line line(currentPosition, velocity2);
          
          [[maybe_unused]] auto const [t1, t2] = *TimeOfIntersection(line, sphere);
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
        
        geometry::Line lineWithoutB(currentPosition, velocity);      
        // determine direction of the particle after adding magnetic field
        // assuming magnetic field does not change during movement
        // First Movement
        auto position = currentPosition + velocity * min / 2;
        // Change of direction by magnetic field
        geometry::Vector<dimensionless_d> const directionAfter = directionBefore + velocity.cross(magneticfield) *
                                                                  min * k;
        // Second Movement
        position = position + directionAfter * velocity.norm() * min / 2;
        if ((position - currentPosition).GetNorm() != 0_m) {
          geometry::Vector<dimensionless_d> const direction = (position - currentPosition).normalized();
          velocity = direction * velocity.norm();
        } // no velocity update for very small steps
        geometry::Line lineWithB(currentPosition, velocity);

        return std::make_tuple(geometry::Trajectory<geometry::Line>(lineWithoutB, min),
                               geometry::Trajectory<geometry::Line>(lineWithB, min),
                               velocity.norm() * min, minIter->second);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process
