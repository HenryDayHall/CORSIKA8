#ifndef _include_corsika_processes_TrackinLine_h_
#define _include_corsika_processes_TrackinLine_h_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/environment/Environment.h>
#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

#include <optional>

#include <iostream>

using namespace corsika;

namespace corsika::process {

  namespace tracking_line {

    template <typename Stack>
    class TrackingLine { //
      typedef typename Stack::ParticleType Particle;

      corsika::environment::Environment const& fEnvironment;

    public:
      std::optional<corsika::units::si::TimeType> TimeOfIntersection(
          geometry::Trajectory<corsika::geometry::Line> const& traj,
          geometry::Sphere const& sphere) {
        using namespace corsika::units::si;
        auto const& cs = fEnvironment.GetCoordinateSystem();
        geometry::Point const origin(cs, 0_m, 0_m, 0_m);

        auto const r0 = (traj.GetR0() - origin);
        auto const v0 = traj.GetV0();
        auto const c0 = (sphere.GetCenter() - origin);

        auto const alpha = r0.dot(v0) - 2 * v0.dot(c0);
        auto const beta = c0.squaredNorm() + r0.squaredNorm() + 2 * c0.dot(r0) -
                          sphere.GetRadius() * sphere.GetRadius();

        auto const discriminant = alpha * alpha - 4 * beta * v0.squaredNorm();

        //~ std::cout << "discriminant: " << discriminant << std::endl;
        //~ std::cout << "alpha: " << alpha << std::endl;
        //~ std::cout << "beta: " << beta << std::endl;

        if (discriminant.magnitude() > 0) {
          auto const t = (-alpha - sqrt(discriminant)) / (2 * v0.squaredNorm());
          return t;
        } else {
          return {};
        }
      }

      TrackingLine(corsika::environment::Environment const& pEnv)
          : fEnvironment(pEnv) {}
      void Init() {}
      auto GetTrack(Particle& p) {
        using namespace corsika::units::si;
        geometry::Vector<SpeedType::dimension_type> const velocity =
            p.GetMomentum() / p.GetEnergy() * corsika::units::si::constants::cSquared;
        geometry::Line traj(p.GetPosition(), velocity);
        return geometry::Trajectory<corsika::geometry::Line>(traj, 100_ns);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
