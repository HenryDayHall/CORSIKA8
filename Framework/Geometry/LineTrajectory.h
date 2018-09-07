#ifndef _include_LINETRAJECTORY_H
#define _include_LINETRAJECTORY_H

#include <fwk/Point.h>
#include <fwk/Vector.h>
#include <Units/PhysicalUnits.h>

namesapce fwk {

class LineTrajectory // TODO: inherit from Trajectory
{
    using SpeedVec = Vector<Speed::dimension_type>;
    
    Point const r0;
    SpeedVec const v0;
    
    LineTrajectory(Point const& pR0, SpeedVec const& pV0) :
        r0(r0), v0(pV0)
    {
    }
    
    auto GetPosition(Time t) const
    {
        return r0 + v0 * t;
    }
};

} // end namesapce

#endif
