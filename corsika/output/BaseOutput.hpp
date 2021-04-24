/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <boost/filesystem>

#include <yaml-cpp/yaml.h>

namespace corsika {

  /**
   * This is the base class for all outputs so that they
   * can be stored in homogeneous containers.
   */
  class BaseOutput {

  protected:
    int shower_{0}; ///< The current event number.

    BaseOutput();

  public:
    /**
     * Called at the start of each run.
     */
    virtual void startOfLibrary(boost::filesystem::path const& directory) = 0;

    /**
     * Called at the start of each event/shower.
     */
    virtual void startOfShower() {}

    /**
     * Called at the end of each event/shower.
     */
    virtual void endOfShower() = 0;

    /**
     * Called at the end of each run.
     */
    virtual void endOfLibrary() = 0;

    /**
     * Get the configuration of this output.
     */
    virtual YAML::Node getConfig() const = 0;

    /**
     * Get any summary information for the entire library.
     */
    virtual YAML::Node getSummary() { return YAML::Node(); };
  };

} // namespace corsika

#include <corsika/detail/output/BaseOutput.inl>
