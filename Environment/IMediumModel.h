#ifndef _include_IMediumModel_h
#define _include_IMediumModels_h

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/units/PhysicalUnits.h>
#include <tuple>
#include <vector>

namespace corsika::environment {

  class IMediumModel {
  public:
    virtual ~IMediumModel() = default;

    virtual corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const&) const = 0;
    virtual corsika::units::si::GrammageType IntegratedGrammage(BaseTrajectory const&,
                                                                double, double) const = 0;
    virtual NuclearComposition const& GetNuclearComposition() const = 0;
  };

} // namespace corsika::environment

#endif
