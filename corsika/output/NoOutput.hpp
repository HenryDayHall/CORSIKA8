/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/output/BaseOutput.hpp>

namespace corsika {

  /**
   * This class can be used as a drop-in for any output template
   * and doesn't write *any* output files.
   *
   */
  class NoOutput : public BaseOutput {

  public:
    /**
     * Construct a blank output.
     */
    NoOutput();

    /**
     * Called at the start of each run.
     */
    virtual void startOfRun(std::filesystem::path const& directory) final override;

    /**
     * Called at the start of each event/shower.
     */
    virtual void startOfEvent() final override;

    /**
     * Called at the end of each event/shower.
     */
    virtual void endOfEvent() final override;

    /**
     * Called at the end of each run.
     */
    virtual void endOfRun() final override;

    /**
     * Get the configuration of this output.
     */
    virtual YAML::Node getConfig() const final override;

    /**
     * Accept any arguments and ignore them.
     */
    template <typename... TVArgs>
    void write(TVArgs&& args...);

    /**
     * Get final text outputs for the config file.
     */
    virtual YAML::Node getFinalOutput() const final override;
  };

} // namespace corsika

#include <corsika/detail/output/NoOutput.inl>
