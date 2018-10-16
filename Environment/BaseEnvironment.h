#ifndef _include_EnvironmentBase_h
#define _include_EnvironmentBase_h

#include <vector>
#include <memory>
#include <algorithm>
#include <corsika/geometry/Sphere.h>
#include <corsika/geometry/Point.h>

namespace corsika::environment {

class BaseEnvironment {
    std::vector<std::unique_ptr<BaseEnvironment>> childNodes, excludedOverlapNodes;
    corsika::geometry::Sphere shape;
    
public:
    bool ShapeContains(corsika::geometry::Point const& p) const {
        return shape.Contains(p);
    }

    BaseEnvironment* GetContainingNode(corsika::geometry::Point const& p) const {
        if (!shape.Contains(p))
        {
            return nullptr;
        }
            
        auto const predicate = [&] (auto const& s) {return bool(s->GetContainingNode(p));};
        
        auto const childFountIt = std::find_if(childNodes.cbegin(), childNodes.cend(), predicate);
        
        if (childFoundIt == childNodes.cend())
        {
            if (auto const excludedIt = std::find_if(excludedOverlapNodes.cbegin(), excludedOverlapNodes.cend(), predicate); excluded == excludedOverlapNodes.cend())
            {
                return *this;
            }
            else
            {
                return excludedIt;
            }
        }
        
        return (*childFoundIt)->GetContainingNode(p);
    }

    /**
     * Get mass density at given point \a p. It is the caller's responsibility
     * to call this method from the appropriate node in whose domain \a p is located.
     */
    virtual MassDensityType GetDensity(corsika::geometry::Point const& p) = 0;
    
    virtual std::pair<corsika::particles::Code const*, corsika::particles::Code const*> GetTargetComposition(Point const& p)
};

}

#endif
