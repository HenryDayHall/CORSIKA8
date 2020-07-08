/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <corsika/process/conex/CONEX.h>

#include <corsika/random/RNGManager.h>

#include <corsika/particles/ParticleProperties.h>

#include <corsika/geometry/Point.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/utl/CorsikaFenv.h>
#include <catch2/catch.hpp>

TEST_CASE("CONEX", "[processes]") {

  SECTION("linking conex") {
    using std::cout;
    using std::endl;

    std::string parameterPathName = "";
    //auto cxModel = eSibyll23;
    //ConexDynamicInterface cx(cxModel);

    int randomSeeds[3];
    randomSeeds[0] = 1234;
    randomSeeds[1] = 0;
    randomSeeds[2] = 0;

    int nShower = 1; // large to avoid final stats.
    int maxDetail = 0;
    int particleListMode = 0;
    //cx.Init(nShower, randomSeeds, maxDetail, particleListMode, parameterPathName);

    double energyInGeV = 100.;
    double zenith = 60;
    double azimuth = 0;
    double impactParameter = 0;
    int particleType = 100;

    //cx.RunConex(randomSeeds, energyInGeV, zenith, azimuth, impactParameter, particleType);
  }
}
