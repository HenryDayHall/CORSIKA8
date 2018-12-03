#ifndef _include_HomogeneousMedium_h_
#define _include_HomogeneousMedium_h_

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/geometry/Point.h>
#include <corsika/particles/ParticleProperties.h>
#include <corsika/units/PhysicalUnits.h>

/**
 * a homogeneous medium
 */

namespace corsika::environment {

  template <class T>
  class HomogeneousMedium : T {
    corsika::units::si::MassDensityType const fDensity;
    NuclearComposition const fNuclComp;

  public:
    HomogeneousMedium(corsika::units::si::MassDensityType pDensity,
                      NuclearComposition pNuclComp)
        : fDensity(pDensity)
        , fNuclComp(pNuclComp){};

    corsika::units::si::MassDensityType GetMassDensity([
        [maybe_unused]] corsika::geometry::Point const& p) const override {
      return fDensity;
    }
    NuclearComposition const& GetNuclearComposition() const override { return fNuclComp; }

    corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::BaseTrajectory const& pTraj,
        corsika::units::si::TimeType pTo) const override {
      using namespace corsika::units::si;
      return pTraj.DistanceBetween(0_s, pTo) * fDensity;
    }

    corsika::units::si::TimeType FromGrammage(
        corsika::geometry::BaseTrajectory const& pTraj,
        corsika::units::si::GrammageType pGrammage) const override {
      return pTraj.TimeFromArclength(pGrammage / fDensity);
    }
  };

} // namespace corsika::environment
#endif
