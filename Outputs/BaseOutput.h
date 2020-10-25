/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <memory>

#include <filesystem>

#include <yaml-cpp/yaml.h>

namespace corsika::output {

  /**
   * This is the base class for all outputs so that they
   * can be stored in homogeneous containers.
   */
  class BaseOutput : public std::enable_shared_from_this<BaseOutput> {

    int const event_{0}; ///< The current event number.

  protected:
    BaseOutput()
        : event_(0){};

  public:
    /**
     * Called at the start of each run.
     */
    virtual void StartOfRun(std::filesystem::path const& directory) = 0;

    /**
     * Called at the start of each event/shower.
     */
    virtual void StartOfEvent() = 0;

    /**
     * Called at the end of each event/shower.
     */
    virtual void EndOfEvent() = 0;

    /**
     * Called at the end of each run.
     */
    virtual void EndOfRun() = 0;

    /**
     * Get the configuration of this output.
     */
    virtual YAML::Node GetConfig() const = 0;

    /**
     * Get final text outputs for the config file.
     */
    virtual YAML::Node GetOutput() = 0;
  };

} // namespace corsika::output
