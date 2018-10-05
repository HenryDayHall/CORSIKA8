#ifndef _include_HydrogenSphere_h_
#define _include_HydrogenSphere_h_

#include <corsika/geometry/CoordinateSystem.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

/**
 * a fSphere homogeneously filled with hydrogen
 */

namespace corsika::environment {

class HydrogenSphere {
  CoordinateSystem const& fCS;
  corsika::geometry::Sphere const fSphere;
  MassDensityType const fDensity;

public:
  HydrogenSphere(corsika::geometry::CoordinateSystem const& pEnvCS, LengthType pRadius,
                 MassDensityType pDensity)
      : fCS(pEnvCS) fSphere(corsika::geometry::Point(pEnvCS, {0_m, 0_m, 0_m}), radius)
      , fDensity(pDensity) {}

  auto GetTargetParticle(Point const& p) const {
    return fSphere.isInside(p) ? corsika::particles::Code::Proton
                               : corsika::particles::Code::Unknown;
  }

  MassDensityType GetDensity(Point const& p) const { return fSphere.isInside(p) ? density : 0_kg / (meter*meter*meter); };

  GetMagneticField(Point const& p) {
    QuantityVector<magnetic_flux_density_d> components{0 * corsika::units::tesla,
                                                       0. * corsika::units::tesla,
                                                       0. * corsika::units::tesla};
    return Vector<magnetic_flux_density_d>(fCS, components);
  }
};

}
#endif
