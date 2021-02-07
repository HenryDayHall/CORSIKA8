/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <chrono>
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
      NoInit,
      LibraryReady,
      ShowerInProgress,
      LibraryFinished,
    };

    OutputState state_{OutputState::NoInit}; ///< The current state of this manager.
    std::string const name_;                 ///< The name of this simulation file.
    std::filesystem::path const root_;       ///< The top-level directory for the output.
    int count_{0};                           ///< The current ID of this shower.
    std::chrono::time_point<std::chrono::system_clock> const start_time{
        std::chrono::system_clock::now()};           ///< The time the manager is created.
    inline static auto logger{get_logger("output")}; ///< A custom logger.
    /**
     * The outputs that have been registered with us.
     */
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

    /**
     * Write the top-level summary of this library.
     */
    void writeTopLevelSummary() const;

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

    /**
     * Called at the start of each library.
     *
     * This iteratively calls startOfLibrary on each registered output.
     */
    void startOfLibrary();

    /**
     * Called at the start of each event/shower.
     * This iteratively calls startOfEvent on each registered output.
     */
    void startOfShower();

    /**
     * Called at the end of each event/shower.
     * This iteratively calls endOfEvent on each registered output.
     */
    void endOfShower();

    /**
     * Called at the end of each library.
     * This iteratively calls endOfLibrary on each registered output.
     */
    void endOfLibrary();

  }; // class OutputManager

} // namespace corsika

#include <corsika/detail/output/OutputManager.inl>
