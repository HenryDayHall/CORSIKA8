/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

//#include <memory>
#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace corsika {

  /**
   * This is the base class for all outputs so that they
   * can be stored in homogeneous containers.
   */
  // class BaseOutput : public std::enable_shared_from_this<BaseOutput> {
  class BaseOutput {

  protected:
    int event_{0}; ///< The current event number.
    int run_{0};   ///< The current run number.

    BaseOutput();

  public:
    /**
     * Called at the start of each run.
     */
    virtual void startOfRun(std::filesystem::path const& directory) = 0;

    /**
     * Called at the start of each event/shower.
     */
    virtual void startOfEvent() {}

    /**
     * Called at the end of each event/shower.
     */
    virtual void endOfEvent() = 0;

    /**
     * Called at the end of each run.
     */
    virtual void endOfRun() = 0;

    /**
     * Get the configuration of this output.
     */
    virtual YAML::Node getConfig() const = 0;

    /**
     * Get final text outputs for the config file.
     */
    virtual YAML::Node getFinalOutput() { return YAML::Node(); };
  };

} // namespace corsika

#include <corsika/detail/output/BaseOutput.inl>
