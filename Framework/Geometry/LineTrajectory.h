#ifndef _include_LINETRAJECTORY_H
#define _include_LINETRAJECTORY_H

#include <Framework/Geometry/Point.h>
#include <Framework/Geometry/Vector.h>
#include <Units/PhysicalUnits.h>

class LineTrajectory // TODO: inherit from Trajectory
{
    using SpeedVec = Vector<Speed::dimension_type>;
    
    Point const r0;
    SpeedVec const v0;
    
    LineTrajectory(Point const& pR0, SpeedVec const& pV0) :
        r0(r0), v0(pV0)
    {
    }
    
    auto getPosition(Time t) const
    {
        return r0 + v0 * t;
    }
};

#endif
