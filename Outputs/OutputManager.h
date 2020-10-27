/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <algorithm>
#include <fstream>
#include <functional>

#include <corsika/logging/Logging.h>
#include <corsika/output/BaseOutput.h>

namespace corsika::output {

  /*!
   * Manage random number generators.
   */
  class OutputManager final {

    /**
     * Indicates the current state of this manager.
     */
    enum class OutputState {
      RunNoInit,
      RunInitialized,
      RunInProgress,
      RunFinished,
    };

    OutputState state_{OutputState::RunNoInit}; ///< The current state of this manager.
    std::string const name_;                    ///< The name of this simulation file.
    std::filesystem::path const root_; ///< The top-level directory for the output.
    inline static auto logger{logging::GetLogger("output")}; ///< A custom logger.

    /**
     * The outputs that have been registered with us.
     */
    // std::map<std::string, std::shared_ptr<BaseOutput>> outputs_;
    std::map<std::string, std::reference_wrapper<BaseOutput>> outputs_;

    /**
     * Write a YAML-node to a file.
     */
    void WriteNode(YAML::Node const& node, std::filesystem::path const& path) const {

      // construct a YAML emitter for this config file
      YAML::Emitter out;

      // and write the node to the output
      out << node;

      // open the output file - this is <output name>.json
      std::ofstream file(path.string());

      // dump the JSON to the file
      file << out.c_str() << std::endl;

      // and close the efile
      file.close();
    }

    /**
     * Write the top-level config of this simulation.
     */
    void WriteTopLevelConfig() const {

      YAML::Node config;

      // some basic info
      config["name"] = name_;               // the simulation name
      config["creator"] = "CORSIKA8";       // a tag to identify C8 libraries
      config["version"] = "8.0.0-prealpha"; // the current version

      // write the node to a file
      WriteNode(config, root_ / ("config.yaml"));
    }

    void InitOutput(std::string const& name) const {
      // construct the path to this directory
      auto const path{root_ / name};

      // create the directory for this process.
      std::filesystem::create_directory(path);

      // write the config for this output to the file
      WriteNode(outputs_.at(name).get().GetConfig(), path / "config.yaml");
    }

  public:
    /**
     * Construct an OutputManager instance with a name in a given directory.
     *
     * @param name    The name of this output collection.
     * @param dir     The directory where the output directory will be stored.
     */
    OutputManager(std::string const& name,
                  std::filesystem::path const& dir = std::filesystem::current_path())
        : name_(name)
        , root_(dir / name) {

      // check if this directory already exists
      if (std::filesystem::exists(root_)) {
        logger->warn(
            "Output directory '{}' already exists! This is currenty not supported.",
            root_.string());
        throw std::runtime_error("Output directory already exists.");
      }

      // construct the directory for this run
      std::filesystem::create_directory(root_);

      // write the top level config file
      WriteTopLevelConfig();

      // we are now initialized
      state_ = OutputState::RunInitialized;
    }

    /**
     * Handle graceful closure of the outputs upon destruction.
     */
    ~OutputManager() {

      // if we are being destructed but EndOfRun() has not been called,
      // make sure that we gracefully close all the outputs
      if (state_ == OutputState::RunInProgress || state_ == OutputState::RunInitialized) {
        EndOfRun();
      }
    }

    /**
     * Register an existing output to this manager.
     *
     * @param name    The unique name of this output.
     * @param args... These are perfect forwarded to the
     *                constructor of the output.
     */
    template <typename TOutput>
    void Register(std::string const name, TOutput& output) {

      // check if that name is already in the map
      if (outputs_.count(name) > 0) {
        logger->warn("'{}' is already registered. All outputs must have unique names.",
                     name);
        return;
      }

      // if we get here, the name is not already in the map
      // so we create the output and register it into the map
      outputs_.insert(std::make_pair(name, std::ref(output)));

      // and initialize this output
      InitOutput(name);
    }

    /**
     * Called at the start of each run.
     */
    void StartOfRun() {
      for (auto& [name, output] : outputs_) {

        // construct the path to this output subdirectory
        auto const path{root_ / name};

        // and start the run
        output.get().StartOfRun(path);
      }

      // we have now started running
      state_ = OutputState::RunInProgress;
    };

    /**
     * Called at the start of each event/shower.
     */
    void StartOfEvent() {

      // if this is called but we are still in the initialized state,
      // make sure that we transition to RunInProgress
      if (state_ == OutputState::RunInitialized) { StartOfRun(); }

      // now start the event for all the outputs
      for (auto& [name, output] : outputs_) { output.get().StartOfEvent(); }
    }

    /**
     * Called at the end of each event/shower.
     */
    void EndOfEvent() {

      for (auto& [name, output] : outputs_) { output.get().EndOfEvent(); }
    }

    /**
     * Called at the end of each run.
     */
    void EndOfRun() {
      for (auto& [name, output] : outputs_) {
        output.get().EndOfRun();
      }

      // and the run has finished
      state_ = OutputState::RunFinished;

      // write any final state information into the config files
      // for (auto& [name, output] : outputs_) { output.get().EndOfRun(); }
    }

  }; // class OutputManager

} // namespace corsika::output
