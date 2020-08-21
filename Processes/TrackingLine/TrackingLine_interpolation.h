/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_processes_TrackingLine_h_
#define _include_corsika_processes_TrackingLine_h_

#include <corsika/geometry/Line.h>
#include <corsika/geometry/Plane.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Trajectory.h>
#include <corsika/geometry/Vector.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/utl/quartic.h>
#include <cmath>
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
      TrackingLine(){};

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
        
        // determine velocity after adding magnetic field
        auto const* currentLogicalVolumeNode = p.GetNode();
        int chargeNumber;
        if(corsika::particles::IsNucleus(p.GetPID())) {
        	chargeNumber = p.GetNuclearZ();
        } else {
        	chargeNumber = corsika::particles::GetChargeNumber(p.GetPID());
        }
        geometry::Vector<dimensionless_d> const directionBefore = velocity.normalized();
        auto magMaxLength = 1_m/0;
        auto directionAfter = directionBefore;
        if(chargeNumber != 0) {
        	auto magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(currentPosition);
        	std::cout << "TrackingLine   B: " << magneticfield.GetComponents() / 1_uT << " uT " << std::endl;
        	geometry::Vector<SpeedType::dimension_type> const velocityVerticalMag = velocity -
        		velocity.parallelProjectionOnto(magneticfield);
		LengthType const gyroradius = p.GetEnergy() * velocityVerticalMag.GetNorm() * 1_V / 
					      (corsika::units::constants::cSquared * abs(chargeNumber) * 
					      magneticfield.GetNorm() * 1_eV);
 		//steplength should consider more things than just gyroradius
    double maxAngle = 1e-5; 
		LengthType const Steplength = 2 * sin(maxAngle * M_PI / 180) * gyroradius;
   
   std::cout << "Test: " << Steplength << " " << gyroradius << std::endl;
   
		// First Movement
		auto position = currentPosition + directionBefore * Steplength / 2;
		// Change of direction by magnetic field at position
		magneticfield = currentLogicalVolumeNode->GetModelProperties().GetMagneticField(position);
		directionAfter = directionBefore + directionBefore.cross(magneticfield) * chargeNumber * 
				 Steplength * corsika::units::constants::cSquared * 1_eV / 
				 (p.GetEnergy() * velocity.GetNorm() * 1_V); 
		// Second Movement
		position = position + directionAfter * Steplength / 2;
		magMaxLength = (position - currentPosition).GetNorm();
		geometry::Vector<dimensionless_d> const direction = (position - currentPosition) / 
									magMaxLength;
		velocity = direction * velocity.GetNorm();
		std::cout << "TrackingLine   p: " << (direction * p.GetMomentum().GetNorm()).GetComponents() / 1_GeV
                  << " GeV " << std::endl;
            
        } else {
        	std::cout << "TrackingLine   p: " << p.GetMomentum().GetComponents() / 1_GeV
                  << " GeV " << std::endl;
        }
        
        std::cout << "TrackingLine   v: " << velocity.GetComponents() << std::endl;
        
        geometry::Line line(currentPosition, velocity);

        //auto const* currentLogicalVolumeNode = p.GetNode();
        //~ auto const* currentNumericalVolumeNode =
        //~ fEnvironment.GetUniverse()->GetContainingNode(currentPosition);
        auto const numericallyInside =
            currentLogicalVolumeNode->GetVolume().Contains(currentPosition);

        std::cout << "numericallyInside = " << (numericallyInside ? "true" : "false");

        auto const& children = currentLogicalVolumeNode->GetChildNodes();
        auto const& excluded = currentLogicalVolumeNode->GetExcludedNodes();

        std::vector<std::pair<TimeType, decltype(p.GetNode())>> intersections;

        // for entering from outside
        auto addIfIntersects = [&](auto const& vtn) {
          auto const& volume = vtn.GetVolume();
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              volume); // for the moment we are a bit bold here and assume
          // everything is a sphere, crashes with exception if not
          
          std::cout << "Mittelpunkt: " << sphere.GetCenter().GetCoordinates() << std::endl;

          if (auto opt = TimeOfIntersection(line, sphere); opt.has_value()) {
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
        };

        for (auto const& child : children) { addIfIntersects(*child); }
        for (auto const* ex : excluded) { addIfIntersects(*ex); }

        {
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              currentLogicalVolumeNode->GetVolume());
          // for the moment we are a bit bold here and assume
          // everything is a sphere, crashes with exception if not
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

        return std::make_tuple(geometry::Trajectory<geometry::Line>(line, min),
                               velocity.norm() * min, minIter->second, magMaxLength, 
                               directionBefore, directionAfter);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
