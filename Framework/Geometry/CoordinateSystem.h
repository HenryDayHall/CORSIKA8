
/**
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_COORDINATESYSTEM_H_
#define _include_COORDINATESYSTEM_H_

#include <corsika/geometry/QuantityVector.h>
#include <corsika/units/PhysicalUnits.h>
#include <Eigen/Dense>

typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
typedef Eigen::Translation<double, 3> EigenTranslation;

namespace corsika::geometry {

  class RootCoordinateSystem;
  
  using corsika::units::si::length_d;

  class CoordinateSystem {
    CoordinateSystem const* reference = nullptr;
    EigenTransform transf;

    CoordinateSystem(CoordinateSystem const& reference, EigenTransform const& transf)
        : reference(&reference)
        , transf(transf) {}
        
    CoordinateSystem()
        : // for creating the root CS
        transf(EigenTransform::Identity()) {}

  protected:
	static auto CreateCS() { return CoordinateSystem(); }
	friend corsika::geometry::RootCoordinateSystem; /// this is the only class that can creat ONE unique root CS

  public:
  
    static EigenTransform GetTransformation(CoordinateSystem const& c1,
                                            CoordinateSystem const& c2);

    auto& operator=(const CoordinateSystem& pCS) {
      reference = pCS.reference;
      transf = pCS.transf;
      return *this;
    }

    auto translate(QuantityVector<length_d> vector) const {
      EigenTransform const translation{EigenTranslation(vector.eVector)};

      return CoordinateSystem(*this, translation);
    }

    auto rotate(QuantityVector<phys::units::length_d> axis, double angle) const {
      if (axis.eVector.isZero()) {
          throw std::string("null-vector given as axis parameter");
      }
      
      EigenTransform const rotation{Eigen::AngleAxisd(angle, axis.eVector.normalized())};

      return CoordinateSystem(*this, rotation);
    }

    auto translateAndRotate(QuantityVector<phys::units::length_d> translation,
                            QuantityVector<phys::units::length_d> axis, double angle) {
      if (axis.eVector.isZero()) {
          throw std::string("null-vector given as axis parameter");
      }
                                
      EigenTransform const transf{Eigen::AngleAxisd(angle, axis.eVector.normalized()) *
                                  EigenTranslation(translation.eVector)};

      return CoordinateSystem(*this, transf);
    }

    auto const* GetReference() const { return reference; }

    auto const& GetTransform() const { return transf; }
  };

} // namespace corsika::geometry

#endif
