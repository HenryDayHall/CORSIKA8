/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <string>
#include <filesystem>
#include <corsika/output/BaseOutput.hpp>
#include <corsika/framework/core/Logging.hpp>

namespace corsika {

  /*!
   * Manages CORSIKA 8 output streams.
   */
  class OutputManager final {

    /**
     * Indicates the current state of this manager.
     */
    enum class OutputState {
      RunNoInit,
      RunInitialized,
      EventInProgress,
      RunFinished,
    };

    OutputState state_{OutputState::RunNoInit}; ///< The current state of this manager.
    std::string const name_;                    ///< The name of this simulation file.
    std::filesystem::path const root_; ///< The top-level directory for the output.
    inline static auto logger{get_logger("output")}; ///< A custom logger.

    /**
     * The outputs that have been registered with us.
     */
    // std::map<std::string, std::shared_ptr<BaseOutput>> outputs_;
    std::map<std::string, std::reference_wrapper<BaseOutput>> outputs_;

    /**
     * Write a YAML-node to a file.
     */
    void writeNode(YAML::Node const& node, std::filesystem::path const& path) const;

    /**
     * Write the top-level config of this simulation.
     */
    void writeTopLevelConfig() const;

    /**
     * Initialize the "registered" output with a given name.
     */
    void initOutput(std::string const& name) const;

  public:
    /**
     * Construct an OutputManager instance with a name in a given directory.
     *
     * @param name    The name of this output collection.
     * @param dir     The directory where the output directory will be stored.
     */
    OutputManager(std::string const& name, std::filesystem::path const& dir);

    /**
     * Handle graceful closure of the outputs upon destruction.
     */
    ~OutputManager();

    /**
     * Register an existing output to this manager.
     *
     * @param name    The unique name of this output.
     * @param args... These are perfect forwarded to the
     *                constructor of the output.
     */
    template <typename TOutput>
    void add(std::string const& name, TOutput& output);
    // void add(std::string const& name, BaseOutput& output);

    /**
     * Called at the start of each run.
     *
     * This iteratively calls startOfRun on each registered output.
     */
    void startOfRun();

    /**
     * Called at the start of each event/shower.
     * This iteratively calls startOfEvent on each registered output.
     */
    void startOfEvent();

    /**
     * Called at the end of each event/shower.
     * This iteratively calls endOfEvent on each registered output.
     */
    void endOfEvent();

    /**
     * Called at the end of each run.
     * This iteratively calls endOfRun on each registered output.
     */
    void endOfRun();

  }; // class OutputManager

} // namespace corsika

#include <corsika/detail/output/OutputManager.inl>
