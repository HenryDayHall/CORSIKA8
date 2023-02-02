/*
 * (c) Copyright 2023 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/modules/fluka/InteractionModel.hpp>
//~ #include <SetupTestEnvironment.hpp>

#include <corsika/framework/geometry/Point.hpp>
#include <corsika/framework/geometry/CoordinateSystem.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/IMediumModel.hpp>
#include <corsika/media/HomogeneousMedium.hpp>

#include <catch2/catch.hpp>

using namespace corsika;

TEST_CASE("FLUKA") {
    using DummyEnvironmentInterface = IMediumModel;
    using DummyEnvironment = Environment<DummyEnvironmentInterface>;
    using MyHomogeneousModel = HomogeneousMedium<DummyEnvironmentInterface>;
    
    DummyEnvironment env;
    auto& universe = *env.getUniverse();
    CoordinateSystemPtr const& cs = env.getCoordinateSystem();
    universe.setModelProperties<MyHomogeneousModel>(
          1_kg / (1_m * 1_m * 1_m),
          NuclearComposition(std::vector<Code>{Code::Hydrogen, Code::Oxygen, Code::Nitrogen, Code::Argon},
          std::vector<double>{1./4., 1./4., 1./4., 1./4.}));

    corsika::fluka::InteractionModel flukaModel{env};
    
}
