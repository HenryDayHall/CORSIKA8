/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/Environment.h>
#include <corsika/environment/IMediumModel.h>
#include <corsika/environment/NuclearComposition.h>
#include <corsika/environment/VolumeTreeNode.h>
#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <memory>
#include <stack>

namespace corsika::environment {

  /**
   * A namespace containing various Earth radii.
   */
  namespace EarthRadius {
    static constexpr auto Mean{6'371'000 * units::si::meter};
    static constexpr auto Eqautorial{6'378'137 * units::si::meter};
    static constexpr auto Polar{6'356'752 * units::si::meter};
    static constexpr auto PolarCurvature{6'399'593 * units::si::meter};
  } // namespace EarthRadius

  class LayeredSphericalAtmosphereBuilder {
    std::unique_ptr<NuclearComposition> composition_;
    geometry::Point center_;
    units::si::LengthType previousRadius_{units::si::LengthType::zero()};
    units::si::LengthType earthRadius_;

    std::stack<VolumeTreeNode<environment::IMediumModel>::VTNUPtr>
        layers_; // innermost layer first

    void checkRadius(units::si::LengthType) const;

  public:
    LayeredSphericalAtmosphereBuilder(
        corsika::geometry::Point center,
        units::si::LengthType earthRadius = EarthRadius::Mean)
        : center_(center)
        , earthRadius_(earthRadius) {}

    void setNuclearComposition(NuclearComposition);

    void addExponentialLayer(units::si::GrammageType, units::si::LengthType,
                             units::si::LengthType);

    auto size() const { return layers_.size(); }

    void addLinearLayer(units::si::LengthType, units::si::LengthType);

    void assemble(Environment<IMediumModel>&);
    Environment<IMediumModel> assemble();

    /**
     * Get the current Earth radius.
     */
    units::si::LengthType getEarthRadius() const { return earthRadius_; };
  };

} // namespace corsika::environment
