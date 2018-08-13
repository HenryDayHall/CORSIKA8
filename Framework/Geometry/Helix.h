#ifndef _include_HELIX_H_
#define _include_HELIX_H_

#include <Geometry/Vector.h>
#include <Geometry/Point.h>
#include <Units/PhysicalUnits.h>

class Helix // TODO: inherit from to-be-implemented "Trajectory"
{
    using SpeedVec = Vector<phys::units::speed_d>;
    
    Point const r0;
    phys::units::quantity<phys::units::frequency_d> const omegaC;
    SpeedVec const vPar;
    SpeedVec vPerp, uPerp;
    
public:
    Helix(Point const pR0, phys::units::quantity<phys::units::frequency_d> pOmegaC,
        SpeedVec const pvPar, SpeedVec const pvPerp) :
        r0(pR0), omegaC(pOmegaC), vPar(pvPar), vPerp(pvPerp), uPerp(vPar.normalized().cross(vPerp))
    {
        
    }
    
    Point getPosition(phys::units::quantity<phys::units::time_interval_d> t) const
    {
        return r0 + vPar * t + (vPerp * (cos(omegaC * t) - 1) + uPerp * sin(omegaC * t)) / omegaC;
    }
};

#endif
