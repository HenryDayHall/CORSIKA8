#ifndef _include_HomogeneousMedium_h_
#define _include_HomogeneousMedium_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

/**
 * a homogeneous medium
 */

namespace corsika::environment {

  template <class T>
  class HomogeneousMedium : T {
    MassDensityType const fDensity;
    NuclearComposition const fNuclComp;

  public:
    HomogeneousMedium(MassDensityType pDensity, NuclearComposition pNuclComp)
        : fDensity(pDensity)
        , fNuclComp(pNuclComp){};

    MassDensityType GetMassDensity(Point const& p) const override { return fDensity; }
    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }
  };

} // namespace corsika::environment
#endif
