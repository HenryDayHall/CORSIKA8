#ifndef _include_IMediumModel_h
#define _include_IMediumModels_h

#include <corsika/environment/NuclearComposition.h>
#include <corsika/geometry/BaseTrajectory.h>
#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>
#include <tuple>
#include <vector>

namespace corsika::environment {

  class IMediumModel {
  public:
    virtual ~IMediumModel() = default;

    virtual corsika::units::si::MassDensityType GetMassDensity(
        corsika::geometry::Point const&) const = 0;
    virtual corsika::units::si::GrammageType IntegratedGrammage(
        corsika::geometry::BaseTrajectory const&, corsika::units::si::TimeType) const = 0;
    virtual corsika::units::si::TimeType FromGrammage(
        corsika::geometry::BaseTrajectory const&,
        corsika::units::si::GrammageType) const = 0;
    virtual NuclearComposition const& GetNuclearComposition() const = 0;
  };

} // namespace corsika::environment

#endif
