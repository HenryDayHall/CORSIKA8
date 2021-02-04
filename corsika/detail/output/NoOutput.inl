/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

namespace corsika {

  NoOutput::NoOutput()
      : {}

  void NoOutput::startOfRun(std::filesystem::path const& directory) {}

  void NoOutput::startOfEvent() {}

  void NoOutput::endOfEvent() {}

  void NoOutput::endOfRun() {}

  YAML::Node NoOutput::getConfig() const { return YAML::Node(); }

  YAML::Node NoOutput::getFinalOutput() const { return YAML::Node(); }

  template <typename... TVArgs>
  void NoOutput::write(TVArgs&& args...) {}

} // namespace corsika
