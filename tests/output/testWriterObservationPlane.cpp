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

  TestWriterPlane(bool const printZ)
      : ParticleWriterParquet(printZ) {}

  YAML::Node getConfig() const { return YAML::Node(); }

  void checkWrite() {
    ParticleWriterParquet::write(Code::Unknown, 1_GeV, 2_m, 3_m, 0_m, 1, 0, 0, 1_us, 1.0);
    ParticleWriterParquet::write(Code::Proton, 1_GeV, 2_m, 3_m, 0_m, -1, 0, 0, 1_us, 1.0);
    ParticleWriterParquet::write(Code::MuPlus, 1_GeV, 2_m, 3_m, 0_m, 0, 1, 0, 1_us, 1.0);
    ParticleWriterParquet::write(Code::MuMinus, 1_GeV, 2_m, 3_m, 0_m, 0, -1, 0, 1_us,
                                 1.0);
    ParticleWriterParquet::write(Code::Photon, 1_GeV, 2_m, 3_m, 0_m, 0, 0, -1, 1_us, 1.0);
  }
};

TEST_CASE("ObservationPlaneWriterParquet") {

  logging::set_level(logging::level::info);

  SECTION("standard") {

    std::string const file_dir = "./output_dir_obs_plane";

    // preparation
    if (boost::filesystem::exists(file_dir)) { boost::filesystem::remove_all(file_dir); }
    boost::filesystem::create_directory(file_dir);

    TestWriterPlane test(true);
    test.startOfLibrary(file_dir);
    test.startOfShower(0);

    // write a few particles
    test.checkWrite();

    test.endOfShower(0);
    test.endOfLibrary();

    CHECK(boost::filesystem::exists(file_dir + "/particles.parquet"));

    auto const summary = test.getSummary();

    CHECK(summary["Eground"].as<double>() == Approx(5));
    CHECK(summary["hadrons"].as<int>() == Approx(1));
    CHECK(summary["muons"].as<int>() == Approx(2));
    CHECK(summary["em"].as<int>() == Approx(1));
    CHECK(summary["others"].as<int>() == Approx(1));

    // clean things up
    if (boost::filesystem::exists(file_dir)) { boost::filesystem::remove_all(file_dir); }
  }

  SECTION("without_z") {

    std::string const file_dir = "./output_dir_obs_plane_no_z";

    // preparation
    if (boost::filesystem::exists(file_dir)) { boost::filesystem::remove_all(file_dir); }
    boost::filesystem::create_directory(file_dir);

    TestWriterPlane test(false); // do not print the z-coord
    test.startOfLibrary(file_dir);
    test.startOfShower(0);

    // write a few particles
    test.checkWrite();

    test.endOfShower(0);
    test.endOfLibrary();

    CHECK(boost::filesystem::exists(file_dir + "/particles.parquet"));

    auto const summary = test.getSummary();

    CHECK(summary["Eground"].as<double>() == Approx(5));
    CHECK(summary["hadrons"].as<int>() == Approx(1));
    CHECK(summary["muons"].as<int>() == Approx(2));
    CHECK(summary["em"].as<int>() == Approx(1));
    CHECK(summary["others"].as<int>() == Approx(1));

    // clean things up
    if (boost::filesystem::exists(file_dir)) { boost::filesystem::remove_all(file_dir); }
  }
}
