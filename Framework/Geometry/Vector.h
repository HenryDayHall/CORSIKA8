#ifndef _include_VECTOR_H_
#define _include_VECTOR_H_

#include <Geometry/BaseVector.h>
#include <Geometry/QuantityVector.h>
#include <Units/PhysicalUnits.h>

template <typename dim>
class Vector : public BaseVector<dim>
{
    using Quantity = phys::units::quantity<dim, double>;
    
public:
    Vector(CoordinateSystem const& pCS, QuantityVector<dim> pQVector) :
        BaseVector<dim>(pCS, pQVector)
    {
    }

    Vector(CoordinateSystem const& cs, Quantity x, Quantity y, Quantity z) :
        BaseVector<dim>(cs, QuantityVector<dim>(x, y, z))
    {
    }
    
    auto getComponents() const
    {
        return BaseVector<dim>::qVector;
    }
    
    auto getComponents(CoordinateSystem const& pCS) const
    {
        if (&pCS == BaseVector<dim>::cs)
        {
            return BaseVector<dim>::qVector;
        }
        else
        {
            return QuantityVector<dim>(CoordinateSystem::getTransformation(*BaseVector<dim>::cs, pCS).linear() * BaseVector<dim>::qVector.eVector);
        }
    }
    
    void rebase(CoordinateSystem const& pCS)
    {
        BaseVector<dim>::qVector = getComponents(pCS);        
        BaseVector<dim>::cs = &pCS;
    }
    
    auto norm() const
    {
        return BaseVector<dim>::qVector.norm();
    }
    
    auto squaredNorm() const
    {
        return BaseVector<dim>::qVector.squaredNorm();
    }
        
    template <typename dim2>
    auto parallelProjectionOnto(BaseVector<dim2> const& pVec, CoordinateSystem const& pCS) const
    {
        auto const ourCompVec = getComponents(pCS);
        auto const otherCompVec = pVec.getComponents(pVec);
        auto const& a = ourCompVec.eVector;
        auto const& b = otherCompVec.eVector;
        
        return Vector<dim>(pCS, (a * b) / b.squaredNorm() * b);
    }
    
    template <typename dim2>
    auto parallelProjectionOnto(BaseVector<dim2> const& pVec)
    {
        return parallelProjectionOnto<dim2>(pVec, *BaseVector<dim>::cs);
    }
    
    //~ template <typename dim2>
    //~ auto operator*(Vector<dim2> const& pVec)
    //~ {
        //~ auto constexpr resulting_dim = dimension 
        //~ return Vector<
    //~ }
};

#endif
