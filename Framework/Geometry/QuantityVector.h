#ifndef _include_QUANTITYVECTOR_H_
#define _include_QUANTITYVECTOR_H_

#include <Units/PhysicalUnits.h>

#include <Eigen/Dense>
#include <iostream>

template <typename dim>
class QuantityVector
{
protected:
    using Quantity = phys::units::quantity<dim, double>;
    using QuantitySquared = decltype(Quantity(phys::units::detail::magnitude_tag, 0) * Quantity(phys::units::detail::magnitude_tag, 0));
    
public:
    Eigen::Vector3d eVector;

    QuantityVector(Quantity a, Quantity b, Quantity c) :
        eVector{a.magnitude(), b.magnitude(), c.magnitude()}
    {
    }
    
    QuantityVector(Eigen::Vector3d pBareVector) :
        eVector(pBareVector)
    {
    }
    
    auto operator[](size_t index) const
    {
        return Quantity(phys::units::detail::magnitude_tag, eVector[index]);
    }
    
    auto norm() const
    {
        return Quantity(phys::units::detail::magnitude_tag, eVector.norm());
    }
    
    auto squaredNorm() const
    {
        return QuantitySquared(phys::units::detail::magnitude_tag, eVector.squaredNorm());
    }
    
    auto operator+(QuantityVector<dim> const& pQVec) const
    {
        return QuantityVector<dim>(eVector + pQVec.eVector);
    }
    
    auto operator-(QuantityVector<dim> const& pQVec) const
    {
        return QuantityVector<dim>(eVector - pQVec.eVector);
    }
    
    //auto operator*(
};

template <typename dim>
auto& operator<<(std::ostream& os, QuantityVector<dim> qv)
{
    os << '(' << qv.eVector(0) << ' ' << qv.eVector(1) << ' ' << qv.eVector(2)
       << ") " << phys::units::to_unit_symbol(phys::units::quantity<dim, double>());
    return os;
}

#endif
