/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <boost/filesystem.hpp>

#include <corsika/framework/core/Logging.hpp>

#include <corsika/output/OutputManager.hpp>
#include <corsika/output/NoOutput.hpp>

using namespace corsika;

struct DummyNoOutput : public NoOutput {
  void check() {
    NoOutput::startOfLibrary("./");
    NoOutput::startOfShower(0);
    NoOutput::endOfShower(0);
    NoOutput::endOfLibrary();
  }
  void checkWrite() { NoOutput::write(Code::Unknown, 1_eV, 1_m, 1_m, 1_ns); }
};

struct DummyOutput : public BaseOutput {

  bool startLibrary_ = false;
  bool startShower_ = false;
  bool endLibrary_ = false;
  bool endShower_ = false;

  void startOfLibrary(boost::filesystem::path const&) override { startLibrary_ = true; }

  void startOfShower(unsigned int const shower = 0) override {
    BaseOutput::startOfShower(shower);
    startShower_ = true;
  }

  void endOfShower(unsigned int const) override { endShower_ = true; }

  void endOfLibrary() override { endLibrary_ = true; }

  YAML::Node getSummary() const final override {
    YAML::Node summary;
    summary["test"] = "test";
    return summary;
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
    test.endLibrary_ = false;
  }

  SECTION("auto-write") {

    // preparation
    if (boost::filesystem::exists("./out_test")) {
      boost::filesystem::remove_all("./out_test");
    }

    // output manager performs nothing, no action, just interface
    OutputManager* output = new OutputManager("check", "./out_test");

    CHECK(boost::filesystem::is_directory("./out_test/check"));

    DummyOutput test;
    output->add("test", test);
    output->startOfLibrary();
    output->startOfShower();

    // check support for closing automatically
    delete output;
    output = 0;

    CHECK(boost::filesystem::exists("./out_test/check/test/summary.yaml"));
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

    CHECK_THROWS(output.endOfLibrary());

    output.startOfLibrary();

    CHECK_THROWS(output.startOfLibrary());

    output.startOfShower();

    CHECK_THROWS(output.startOfLibrary());

    output.endOfShower();

    CHECK_THROWS(output.startOfLibrary());

    output.endOfLibrary();
  }

  SECTION("NoOutput") {
    // this is one of the classes where testing is a bit useless, but we can at least make
    // sure the interface exists.
    DummyNoOutput nothing;

    nothing.check();
    nothing.checkWrite();
  }
}