/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/Line.hpp>
#include <corsika/framework/geometry/Plane.hpp>
#include <corsika/framework/geometry/Sphere.hpp>
#include <corsika/framework/geometry/Trajectory.hpp>
#include <corsika/framework/geometry/Vector.hpp>

#include <optional>
#include <type_traits>
#include <utility>

namespace corsika::tracking_line {

  std::optional<std::pair<TimeType, TimeType>> TimeOfIntersection(Line const&,
                                                                  Sphere const&);

  TimeType TimeOfIntersection(Line const&, Plane const&);

  class TrackingLine {

  public:
    TrackingLine(){};

    template <typename Particle> // was Stack previously, and argument was
                                 // Stack::StackIterator
    auto getTrack(Particle const& p) {
      Vector<SpeedType::dimension_type> const velocity =
          p.getMomentum() / p.getEnergy() * constants::c;

      auto const currentPosition = p.getPosition();
      std::cout << "TrackingLine pid: " << p.getPID()
                << " , E = " << p.getEnergy() / 1_GeV << " GeV" << std::endl;
      std::cout << "TrackingLine pos: " << currentPosition.getCoordinates() << std::endl;
      std::cout << "TrackingLine   E: " << p.getEnergy() / 1_GeV << " GeV" << std::endl;
      std::cout << "TrackingLine   p: " << p.getMomentum().getComponents() / 1_GeV
                << " GeV " << std::endl;
      std::cout << "TrackingLine   v: " << velocity.getComponents() << std::endl;

      // to do: include effect of magnetic field
      Line line(currentPosition, velocity);

      auto const* currentLogicalVolumeNode = p.getNode();
      auto const numericallyInside =
          currentLogicalVolumeNode->getVolume().isInside(currentPosition);

      std::cout << "numericallyInside = " << (numericallyInside ? "true" : "false");

      auto const& children = currentLogicalVolumeNode->getChildNodes();
      auto const& excluded = currentLogicalVolumeNode->getExcludedNodes();

      std::vector<std::pair<TimeType, decltype(p.getNode())>> intersections;

      // for entering from outside
      auto addIfIntersects = [&](auto const& vtn) {
        auto const& volume = vtn.getVolume();
        auto const& sphere = dynamic_cast<Sphere const&>(
            volume); // for the moment we are a bit bold here and assume
        // everything is a sphere, crashes with exception if not

        if (auto opt = TimeOfIntersection(line, sphere); opt.has_value()) {
          auto const [t1, t2] = *opt;
          std::cout << "intersection times: " << t1 / 1_s << "; "
                    << t2 / 1_s
                    // << " " << vtn.getModelProperties().getName()
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
        auto const& sphere =
            dynamic_cast<Sphere const&>(currentLogicalVolumeNode->getVolume());
        // for the moment we are a bit bold here and assume
        // everything is a sphere, crashes with exception if not
        [[maybe_unused]] auto const [t1, t2] = *TimeOfIntersection(line, sphere);
        [[maybe_unused]] auto dummy_t1 = t1;
        intersections.emplace_back(t2, currentLogicalVolumeNode->getParent());
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
                // << " " << minIter->second->getModelProperties().getName()
                << std::endl;

      return std::make_tuple(Trajectory<Line>(line, min), velocity.getNorm() * min,
                             minIter->second);
    }
  };

} // namespace corsika::tracking_line

#include <corsika/detail/modules/TrackingLine.inl>
