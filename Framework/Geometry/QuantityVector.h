#ifndef _include_QUANTITYVECTOR_H_
#define _include_QUANTITYVECTOR_H_

#include <Units/PhysicalUnits.h>

#include <Eigen/Dense>
#include <iostream>
#include <utility>

template <typename dim>
class QuantityVector
{
protected:
    using Quantity = phys::units::quantity<dim, double>;
    //using QuantitySquared = decltype(std::declval<Quantity>() * std::declval<Quantity>());
    
public:
    Eigen::Vector3d eVector;
    
    typedef dim dimension;

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
        using QuantitySquared = decltype(std::declval<Quantity>() * std::declval<Quantity>());
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
    
    template <typename ScalarDim>
    auto operator*(phys::units::quantity<ScalarDim, double> const p) const
    {
        return QuantityVector<typename phys::units::detail::Product<ScalarDim, dim, double, double>::dimension_type>(eVector * p.magnitude());
        // TODO: this function does not work if the result is dimensionless, as
        // dimensionless quantities are "cast" back to plain old double in PhysUnits.
        // Either change PhysUnits, or cover this case with a template specialization?
    }
    
    auto operator*(double const p) const
    {
        return QuantityVector<dim>(eVector * p);
    }
    
    auto& operator*=(double const p)
    {
        eVector *= p;
        return *this;
    }
    
    auto& operator+=(QuantityVector<dim> const& pQVec)
    {
        eVector += pQVec.eVector;
        return *this;
    }
    
    auto& operator-=(QuantityVector<dim> const& pQVec)
    {
        eVector -= pQVec.eVector;
        return *this;
    }
    
    auto& operator-() const
    {
        return QuantityVector<dim>(-eVector);
    }
    
    auto normalized() const
    {
        return (*this) * (1 / norm());
    }
};

template <typename dim>
auto& operator<<(std::ostream& os, QuantityVector<dim> qv)
{
    using Quantity = phys::units::quantity<dim, double>;
    
    os << '(' << qv.eVector(0) << ' ' << qv.eVector(1) << ' ' << qv.eVector(2)
       << ") " << phys::units::to_unit_symbol<dim, double>(Quantity(phys::units::detail::magnitude_tag, 1));
    return os;
}

#endif
