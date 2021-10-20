/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/modules/writers/ObservationPlaneWriterParquet.hpp>

#include <corsika/framework/core/Logging.hpp>
#include <corsika/framework/geometry/QuantityVector.hpp>

using namespace corsika;

struct TestWriterPlane : public ObservationPlaneWriterParquet {

  YAML::Node getConfig() const { return YAML::Node(); }

  void checkWrite() {
    ObservationPlaneWriterParquet::write(Code::Unknown, 1_eV, 2_m, 3_m, 4_ns);
  }
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
    test.startOfShower();
    test.checkWrite();
    test.endOfShower();
    test.endOfLibrary();

    CHECK(boost::filesystem::exists("./output_dir/particles.parquet"));
  }
}
