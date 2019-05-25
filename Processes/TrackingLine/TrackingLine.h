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
#include <corsika/units/PhysicalUnits.h>
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
        geometry::Vector<SpeedType::dimension_type> const velocity =
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

        // to do: include effect of magnetic field
        geometry::Line line(currentPosition, velocity);

        auto const* currentLogicalVolumeNode = p.GetNode();
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
                               velocity.norm() * min, minIter->second);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
