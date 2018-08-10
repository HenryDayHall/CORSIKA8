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
    auto parallelProjectionOnto(Vector<dim2> const& pVec, CoordinateSystem const& pCS) const
    {
        auto const ourCompVec = getComponents(pCS);
        auto const otherCompVec = pVec.getComponents(pCS);
        auto const& a = ourCompVec.eVector;
        auto const& b = otherCompVec.eVector;
        
        return Vector<dim>(pCS, QuantityVector<dim>(b * ((a.dot(b)) / b.squaredNorm())));
    }
    
    template <typename dim2>
    auto parallelProjectionOnto(Vector<dim2> const& pVec) const
    {
        return parallelProjectionOnto<dim2>(pVec, *BaseVector<dim>::cs);
    }
    
    auto operator+(Vector<dim> const& pVec) const
    {
        auto const components = getComponents(*BaseVector<dim>::cs) + pVec.getComponents(*BaseVector<dim>::cs);
        return Vector<dim>(*BaseVector<dim>::cs, components);
    }
    
    auto operator-(Vector<dim> const& pVec) const
    {
        auto const components = getComponents() - pVec.getComponents(*BaseVector<dim>::cs);
        return Vector<dim>(*BaseVector<dim>::cs, components);
    }
    
    auto& operator*=(double const p)
    {
        BaseVector<dim>::qVector *= p;
        return *this;
    }
    
    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const
    {
        using ResQuantity = decltype(BaseVector<dim>::qVector * p);
        if constexpr (std::is_same<ResQuantity, double>::value) // result dimensionless, not a "Quantity" anymore
        {
            return Vector<phys::units::dimensionless_d>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
        }
        else
        {
            using res_dim = typename decltype(BaseVector<dim>::qVector * p)::dimension;
            return Vector<res_dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);        
        }
    }
    
    auto operator*(double const p) const
    {        
        return Vector<dim>(*BaseVector<dim>::cs, BaseVector<dim>::qVector * p);
    }
    
    auto& operator+=(Vector<dim> const& pVec)
    {
        BaseVector<dim>::qVector += pVec.getComponents(*BaseVector<dim>::cs);
        return *this;
    }
    
    auto& operator-=(Vector<dim> const& pVec)
    {
        BaseVector<dim>::qVector -= pVec.getComponents(*BaseVector<dim>::cs);
        return *this;
    }
    
    auto& operator-() const
    {
        return Vector<dim>(*BaseVector<dim>::cs, - BaseVector<dim>::qVector);
    }
    
    auto normalized() const
    {
        return (*this) * (1 / norm());
    }
    
    //~ template <typename dim2>
    //~ auto operator*(Vector<dim2> const& pVec)
    //~ {
        //~ auto constexpr resulting_dim = dimension 
        //~ return Vector<
    //~ }
};

#endif
