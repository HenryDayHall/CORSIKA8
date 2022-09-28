/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/modules/writers/TrackWriterParquet.hpp>

#include <corsika/framework/core/Logging.hpp>
#include <SetupTestEnvironment.hpp>

using namespace corsika;

auto [env, csPtr, nodePtr] = setup::testing::setup_environment(Code::Oxygen);
auto const& cs = *csPtr;
[[maybe_unused]] auto const& env_dummy = env;
[[maybe_unused]] auto const& node_dummy = nodePtr;

struct TestWriterTrack : public TrackWriterParquet {

  YAML::Node getConfig() const { return YAML::Node(); }

  void checkWrite() {
    TrackWriterParquet::write(Code::Unknown, 1_eV, 1.0, {2_m, 3_m, 4_m}, 1_ns,
                              {5_m, 6_m, 7_m}, 0_eV, 2_ns, nodePtr);
  }
};

TEST_CASE("TrackWriterParquet") {

  logging::set_level(logging::level::info);

  SECTION("standard") {

    // preparation
    if (boost::filesystem::exists("./output_dir_tracks")) {
      boost::filesystem::remove_all("./output_dir_tracks");
    }
    boost::filesystem::create_directory("./output_dir_tracks");

    TestWriterTrack test;
    test.startOfLibrary("./output_dir_tracks");
    test.startOfShower(0);
    test.checkWrite();
    test.endOfShower(0);
    test.endOfLibrary();

    CHECK(boost::filesystem::exists("./output_dir_tracks/tracks.parquet"));
  }
}
