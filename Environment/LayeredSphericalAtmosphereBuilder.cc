/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/environment/LayeredSphericalAtmosphereBuilder.h>
#include <corsika/environment/FlatExponential.h>
#include <corsika/environment/HomogeneousMedium.h>
#include <corsika/environment/SlidingPlanarExponential.h>

using namespace corsika::environment;

void LayeredSphericalAtmosphereBuilder::checkRadius(units::si::LengthType r) const {
  if (r <= previousRadius_) {
    throw std::runtime_error("radius must be greater than previous");
  }
}

void LayeredSphericalAtmosphereBuilder::setNuclearComposition(
    NuclearComposition composition) {
  composition_ = std::make_unique<NuclearComposition>(composition);
}

void LayeredSphericalAtmosphereBuilder::addExponentialLayer(
    units::si::GrammageType b, units::si::LengthType c,
    units::si::LengthType upperBoundary) {
  auto const radius = seaLevel_ + upperBoundary;
  checkRadius(radius);
  previousRadius_ = radius;

  auto node = std::make_unique<VolumeTreeNode<IMediumModel>>(
      std::make_unique<geometry::Sphere>(center_, radius));

  auto const rho0 = b / c;
  std::cout << "rho0 = " << rho0 << ", c = " << c << std::endl;

  node->SetModelProperties<SlidingPlanarExponential<IMediumModel>>(
      center_, rho0, -c, *composition_, seaLevel_);

  layers_.push(std::move(node));
}

void LayeredSphericalAtmosphereBuilder::addLinearLayer(
    units::si::LengthType c, units::si::LengthType upperBoundary) {
  using namespace units::si;

  auto const radius = seaLevel_ + upperBoundary;
  checkRadius(radius);
  previousRadius_ = radius;

  units::si::GrammageType constexpr b = 1 * 1_g / (1_cm * 1_cm);
  auto const rho0 = b / c;

  std::cout << "rho0 = " << rho0;

  auto node = std::make_unique<VolumeTreeNode<environment::IMediumModel>>(
      std::make_unique<geometry::Sphere>(center_, radius));
  node->SetModelProperties<HomogeneousMedium<IMediumModel>>(rho0, *composition_);

  layers_.push(std::move(node));
}

Environment<IMediumModel> LayeredSphericalAtmosphereBuilder::assemble() {
  Environment<IMediumModel> env;
  assemble(env);
  return env;
}

void LayeredSphericalAtmosphereBuilder::assemble(Environment<IMediumModel>& env) {
  auto& universe = env.GetUniverse();
  auto* outmost = universe.get();

  while (!layers_.empty()) {
    auto l = std::move(layers_.top());
    auto* tmp = l.get();
    outmost->AddChild(std::move(l));
    layers_.pop();
    outmost = tmp;
  }
}
