#ifndef _include_HELIX_H_
#define _include_HELIX_H_

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Units/PhysicalUnits.h>

#include <cmath>

class Helix // TODO: inherit from to-be-implemented "Trajectory"
{
    using SpeedVec = Vector<Speed::dimension_type>;
    
    Point const r0;
    Frequency const omegaC;
    SpeedVec const vPar;
    SpeedVec vPerp, uPerp;
    
    Length const radius;
    
public:
    Helix(Point const& pR0, phys::units::quantity<phys::units::frequency_d> pOmegaC,
        SpeedVec const& pvPar, SpeedVec const& pvPerp) :
        r0(pR0), omegaC(pOmegaC), vPar(pvPar), vPerp(pvPerp), uPerp(vPerp.cross(vPar.normalized())),
        radius(pvPar.norm() / abs(pOmegaC))
    {
        
    }
    
    auto GetPosition(Time t) const
    {
        return r0 + vPar * t + (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }
    
    auto GetRadius() const
    {
        return radius;
    }
};

#endif
