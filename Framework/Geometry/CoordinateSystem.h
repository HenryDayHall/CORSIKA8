#ifndef COORDINATESYSTEM_H_
#define COORDINATESYSTEM_H_

#include "QuantityVector.h"
#include <Eigen/Dense>
#include <phys/units/quantity.hpp>

typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
typedef Eigen::Translation<double, 3> EigenTranslation;

class CoordinateSystem
{
    CoordinateSystem const* reference = nullptr;
    EigenTransform transf;
    
    CoordinateSystem(CoordinateSystem const& reference, EigenTransform const& transf) :
        reference(&reference),
        transf(transf)
    {
        
    }
    
public:
    static EigenTransform getTransformation(CoordinateSystem const& c1, CoordinateSystem const& c2);

    CoordinateSystem() : // for creating the root CS
        transf(EigenTransform::Identity())
    {
        
    }
    
    auto& operator=(const CoordinateSystem& pCS)
    {
        reference = pCS.reference;
        transf = pCS.transf;
        return *this;
    }    
    
    auto translate(QuantityVector<phys::units::length_d> vector) const
    {
        EigenTransform const translation{EigenTranslation(vector.eVector)};
        
        return CoordinateSystem(*this, translation);
    }
    
    auto rotate(QuantityVector<phys::units::length_d> axis, double angle) const
    {
        EigenTransform const rotation{Eigen::AngleAxisd(M_PI / 6, axis.eVector.normalized())};
        
        return CoordinateSystem(*this, rotation);
    }
    
    auto translateAndRotate(QuantityVector<phys::units::length_d> translation, QuantityVector<phys::units::length_d> axis, double angle)
    {
        EigenTransform const transf{Eigen::AngleAxisd(M_PI / 6, axis.eVector.normalized()) * EigenTranslation(translation.eVector)};
        
        return CoordinateSystem(*this, transf);
    }
    
    auto const* getReference() const
    {
        return reference;
    }
    
    auto const& getTransform() const
    {
        return transf;
    }    
};

#endif
