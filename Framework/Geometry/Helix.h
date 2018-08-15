#ifndef _include_HELIX_H_
#define _include_HELIX_H_

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Units/PhysicalUnits.h>

class Helix // TODO: inherit from to-be-implemented "Trajectory"
{
    using SpeedVec = Vector<Speed>;
    
    Point const r0;
    Frequency const omegaC;
    SpeedVec const vPar;
    SpeedVec vPerp, uPerp;
    
    Length const radius;
    
public:
    Helix(Point const pR0, phys::units::quantity<phys::units::frequency_d> pOmegaC,
        SpeedVec const pvPar, SpeedVec const pvPerp) :
        r0(pR0), omegaC(pOmegaC), vPar(pvPar), vPerp(pvPerp), uPerp(vPar.normalized().cross(vPerp)),
        radius(pvPar.norm() / pOmegaC)
    {
        
    }
    
    auto getPosition(Time t) const
    {
        return r0 + vPar * t + (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }
    
    auto getRadius() const
    {
        return radius;
    }
};

#endif
