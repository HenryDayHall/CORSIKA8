#ifndef _include_COORDINATESYSTEM_H_
#define _include_COORDINATESYSTEM_H_

#include <corsika/geometry/QuantityVector.h>
#include <corsika/units/PhysicalUnits.h>
#include <Eigen/Dense>

typedef Eigen::Transform<double, 3, Eigen::Affine> EigenTransform;
typedef Eigen::Translation<double, 3> EigenTranslation;

namespace corsika::geometry {

  using corsika::units::length_d;

  class CoordinateSystem {
    CoordinateSystem const* reference = nullptr;
    EigenTransform transf;

    CoordinateSystem(CoordinateSystem const& reference, EigenTransform const& transf)
        : reference(&reference)
        , transf(transf) {}

  public:
    static EigenTransform GetTransformation(CoordinateSystem const& c1,
                                            CoordinateSystem const& c2);

    CoordinateSystem()
        : // for creating the root CS
        transf(EigenTransform::Identity()) {}

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
      EigenTransform const rotation{Eigen::AngleAxisd(angle, axis.eVector.normalized())};

      return CoordinateSystem(*this, rotation);
    }

    auto translateAndRotate(QuantityVector<phys::units::length_d> translation,
                            QuantityVector<phys::units::length_d> axis, double angle) {
      EigenTransform const transf{Eigen::AngleAxisd(angle, axis.eVector.normalized()) *
                                  EigenTranslation(translation.eVector)};

      return CoordinateSystem(*this, transf);
    }

    auto const* GetReference() const { return reference; }

    auto const& GetTransform() const { return transf; }
  };

} // namespace corsika::geometry

#endif
