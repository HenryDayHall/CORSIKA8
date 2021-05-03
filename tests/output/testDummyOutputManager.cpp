/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#include <catch2/catch.hpp>

#include <corsika/output/DummyOutputManager.hpp>
#include <corsika/framework/core/Logging.hpp>

using namespace corsika;

class DummyOutput {};

TEST_CASE("DummyOutputManager", "interface") {

  logging::set_level(logging::level::info);

  // output manager performs nothing, no action, just interface
  DummyOutputManager output;

  DummyOutput test;
  output.add("test", test);

  output.startOfLibrary();
  output.startOfShower();
  output.endOfShower();
  output.endOfLibrary();
}
