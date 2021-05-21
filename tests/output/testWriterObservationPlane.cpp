/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/modules/writers/ParticleWriterParquet.hpp>

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

using namespace corsika;

struct TestWriterPlane : public ParticleWriterParquet {

  YAML::Node getConfig() const { return YAML::Node(); }

  void checkWrite() { ParticleWriterParquet::write(Code::Unknown, 1_eV, 2_m, 3_m, 0_m, 1.0); }
};

TEST_CASE("ObservationPlaneWriterParquet") {

  logging::set_level(logging::level::info);

  SECTION("standard") {

    // preparation
    if (boost::filesystem::exists("./output_dir")) {
      boost::filesystem::remove_all("./output_dir");
    }
    boost::filesystem::create_directory("./output_dir");

    TestWriterPlane test;
    test.startOfLibrary("./output_dir");
    test.startOfShower(0);
    test.checkWrite();
    test.endOfShower(0);
    test.endOfLibrary();

    CHECK(boost::filesystem::exists("./output_dir/particles.parquet"));
  }
}
