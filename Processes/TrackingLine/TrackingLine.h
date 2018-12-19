/**
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

#include <corsika/geometry/Point.h>
#include <corsika/geometry/QuantityVector.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/environment/Environment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <algorithm>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <utility>

using namespace corsika;

namespace corsika::process {

  namespace tracking_line {

    template <typename Stack>
    class TrackingLine { //
      using Particle = typename Stack::ParticleType;

      corsika::environment::Environment const& fEnvironment;

    public:
      std::optional<std::pair<corsika::units::si::TimeType, corsika::units::si::TimeType>>
      TimeOfIntersection(corsika::geometry::Line const& line,
                         geometry::Sphere const& sphere) {
        using namespace corsika::units::si;
        auto const& cs = fEnvironment.GetCoordinateSystem();
        geometry::Point const origin(cs, 0_m, 0_m, 0_m);

        auto const r0 = (line.GetR0() - origin);
        auto const v0 = line.GetV0();
        auto const c0 = (sphere.GetCenter() - origin);

        auto const alpha = r0.dot(v0) - 2 * v0.dot(c0);
        auto const beta = c0.squaredNorm() + r0.squaredNorm() + 2 * c0.dot(r0) -
                          sphere.GetRadius() * sphere.GetRadius();

        auto const discriminant = alpha * alpha - 4 * beta * v0.squaredNorm();

        //~ std::cout << "discriminant: " << discriminant << std::endl;
        //~ std::cout << "alpha: " << alpha << std::endl;
        //~ std::cout << "beta: " << beta << std::endl;

        if (discriminant.magnitude() > 0) {
          (-alpha - sqrt(discriminant)) / (2 * v0.squaredNorm());
          return std::make_pair((-alpha - sqrt(discriminant)) / (2 * v0.squaredNorm()),
                                (-alpha + sqrt(discriminant)) / (2 * v0.squaredNorm()));
        } else {
          return {};
        }
      }

      TrackingLine(corsika::environment::Environment const& pEnv)
          : fEnvironment(pEnv) {}
      void Init() {}

      auto GetTrack(Particle const& p) {
        using std::cout;
        using std::endl;
        using namespace corsika::units::si;
        using namespace corsika::geometry;
        geometry::Vector<SpeedType::dimension_type> const velocity =
            p.GetMomentum() / p.GetEnergy() * corsika::units::constants::c;

        auto const currentPosition = p.GetPosition();

        std::cout << "TrackingLine pos: " << currentPosition.GetCoordinates()
                  << std::endl;
        std::cout << "TrackingLine   v: " << velocity.GetComponents() << std::endl;

        // to do: include effect of magnetic field
        geometry::Line line(currentPosition, velocity);

        auto const* currentVolumeNode =
            fEnvironment.GetUniverse()->GetContainingNode(currentPosition);
        auto const& children = currentVolumeNode->GetChildNodes();
        auto const& excluded = currentVolumeNode->GetExcludedNodes();

        std::vector<TimeType> intersectionTimes;

        auto addIfIntersects = [&](auto& vtn) {
          auto const& volume = vtn.GetVolume();
          auto const& sphere = dynamic_cast<geometry::Sphere const&>(
              volume); // for the moment we are a bit bold here and assume
                       // everything is a sphere, crashes with exception if not

          if (auto opt = TimeOfIntersection(line, sphere); opt.has_value()) {
            auto const [t1, t2] = *opt;
            if (t1.magnitude() >= 0)
              intersectionTimes.push_back(t1);
            else if (t2.magnitude() >= 0)
              intersectionTimes.push_back(t2);
          }
        };

        for (auto const& child : children) { addIfIntersects(*child); }

        for (auto const* child : excluded) { addIfIntersects(*child); }

        addIfIntersects(*currentVolumeNode);

        auto const minIter =
            std::min_element(intersectionTimes.cbegin(), intersectionTimes.cend());

        TimeType min;

        if (minIter == intersectionTimes.cend()) {
          min = 1_s; // todo: do sth. more reasonable as soon as tracking is able
                     // to handle the numerics properly
          //~ throw std::runtime_error("no intersection with anything!");
        } else {
          min = *minIter;
        }

        std::cout << " t-intersect: " << min << std::endl;

        return geometry::Trajectory<corsika::geometry::Line>(line, min);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
