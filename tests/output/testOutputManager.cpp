/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/framework/core/Logging.hpp>

using namespace corsika;

struct DummyOutput : public BaseOutput {

  mutable bool isConfig_ = false;
  mutable bool isSummary_ = false;
  bool startLibrary_ = false;
  bool startShower_ = false;
  bool endLibrary_ = false;
  bool endShower_ = false;

  void startOfLibrary(boost::filesystem::path const&) { startLibrary_ = true; }

  void startOfShower() { startShower_ = true; }

  void endOfShower() { endShower_ = true; }

  void endOfLibrary() { endLibrary_ = true; }

  YAML::Node getConfig() const {
    isConfig_ = true;
    return YAML::Node();
  }

  YAML::Node getSummary() {
    isSummary_ = true;
    return YAML::Node();
  }
};

TEST_CASE("OutputManager") {

  logging::set_level(logging::level::info);

  SECTION("standard") {

    // preparation
    if (boost::filesystem::exists("./out_test")) {
      boost::filesystem::remove_all("./out_test");
    }

    // output manager performs nothing, no action, just interface
    OutputManager output("check", "./out_test");

    CHECK(boost::filesystem::is_directory("./out_test/check"));

    DummyOutput test;
    output.add("test", test);

    CHECK_THROWS(output.add(
        "test",
        test)); // should emit warning which cannot be catched, but no action or failure

    CHECK(test.isConfig_);
    test.isConfig_ = false;

    output.startOfLibrary();
    CHECK(test.startLibrary_);
    test.startLibrary_ = false;

    output.startOfShower();
    CHECK(test.startShower_);
    test.startShower_ = false;

    output.endOfShower();
    CHECK(test.endShower_);
    test.endShower_ = false;

    output.endOfLibrary();
    CHECK(test.endLibrary_);
    CHECK(test.isSummary_);
    test.isSummary_ = false;
    test.endLibrary_ = false;
  }

  SECTION("failures") {

    logging::set_level(logging::level::info);

    // preparation
    if (boost::filesystem::exists("./out_test")) {
      boost::filesystem::remove_all("./out_test");
    }

    // output manager performs nothing, no action, just interface
    OutputManager output("check", "./out_test");
    CHECK_THROWS(new OutputManager("check", "./out_test"));

    // CHECK_THROWS(output.startOfShower());
    // CHECK_THROWS(output.endOfShower());
    CHECK_THROWS(output.endOfLibrary());

    output.startOfLibrary();

    CHECK_THROWS(output.startOfLibrary());
    // CHECK_THROWS(output.endOfShower());
    // CHECK_THROWS(output.endOfLibrary());

    output.startOfShower();

    CHECK_THROWS(output.startOfLibrary());
    // CHECK_THROWS(output.startOfShower());
    // CHECK_THROWS(output.endOfLibrary());

    output.endOfShower();

    CHECK_THROWS(output.startOfLibrary());
    // CHECK_THROWS(output.startOfShower());
    // CHECK_THROWS(output.endOfShower());

    output.endOfLibrary();

    // CHECK_THROWS(output.endOfShower());
    // CHECK_THROWS(output.startOfShower());
    // CHECK_THROWS(output.endOfLibrary());
  }
}