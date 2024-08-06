/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/framework/core/ParticleProperties.hpp>
#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/geometry/RootCoordinateSystem.hpp>

#include <corsika/media/CORSIKA7Atmospheres.hpp>

#include <catch2/catch_all.hpp>

using namespace corsika;
using Catch::Approx;

TEST_CASE("CORSIKA7Atmospheres") {

  logging::set_level(logging::level::info);

  CoordinateSystemPtr const& gCS = get_root_CoordinateSystem();
  Point const gOrigin(gCS, {0_m, 0_m, 0_m});

  Environment<IMediumModel> env;

  // build a Linsley US Standard atmosphere into `env`
  create_5layer_atmosphere<IMediumModel>(env, AtmosphereId::LinsleyUSStd, gOrigin);

  typedef typename Environment<IMediumModel>::BaseNodeType::VTN_type node_type;
  node_type const* universe = env.getUniverse().get();

  Point const p(gCS, {constants::EarthRadius::Mean, 0_m, 0_m});
  CHECK(universe->getContainingNode(p)->getModelProperties().getMassDensity(p) / 1_g *
            static_pow<3>(1_cm) ==
        Approx(1222.6562 / 994186.38));
}
