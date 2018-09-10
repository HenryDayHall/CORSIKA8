#ifndef _include_HELIX_H_
#define _include_HELIX_H_

#include <corsika/geometry/Point.h>
#include <corsika/geometry/Vector.h>
#include <corsika/units/PhysicalUnits.h>
#include <cmath>

namespace corsika::geometry {

  using corsika::units::SpeedType;
  using corsika::units::TimeType;
  using corsika::units::FrequencyType;
  using corsika::units::quantity;
  using corsika::units::frequency_d;
  
  class Helix // TODO: inherit from to-be-implemented "Trajectory"
  {
    using SpeedVec = Vector<SpeedType::dimension_type>;

    Point const r0;
    FrequencyType const omegaC;
    SpeedVec const vPar;
    SpeedVec vPerp, uPerp;

    LengthType const radius;

  public:
    Helix(Point const& pR0, quantity<frequency_d> pOmegaC,
          SpeedVec const& pvPar, SpeedVec const& pvPerp)
        : r0(pR0)
        , omegaC(pOmegaC)
        , vPar(pvPar)
        , vPerp(pvPerp)
        , uPerp(vPerp.cross(vPar.normalized()))
        , radius(pvPar.norm() / abs(pOmegaC)) {}

    auto GetPosition(TimeType t) const {
      return r0 + vPar * t +
             (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }

    auto GetRadius() const { return radius; }
  };

} // namespace corsika

#endif
