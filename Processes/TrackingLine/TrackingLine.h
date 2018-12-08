#ifndef _include_corsika_processes_TrackinLine_h_
#define _include_corsika_processes_TrackinLine_h_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>

#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupStack.h>
#include <corsika/setup/SetupTrajectory.h>

using namespace corsika;

namespace corsika::process {

  namespace tracking_line {

    template <typename Stack>
    class TrackingLine { // Newton-step, naja.. not yet
      typedef typename Stack::ParticleType Particle;

    public:
      void Init() {}
      setup::Trajectory GetTrack(Particle& p) {
        geometry::Vector<SpeedType::dimension_type> v = p.GetDirection();
        geometry::Line traj(p.GetPosition(), v);
        return geometry::Trajectory<corsika::geometry::Line>(traj, 100_ns);
      }
    };

  } // namespace tracking_line

} // namespace corsika::process

#endif
