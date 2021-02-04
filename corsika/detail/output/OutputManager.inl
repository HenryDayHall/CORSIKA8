/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <algorithm>
#include <fstream>
#include <functional>

namespace corsika {

  void OutputManager::writeNode(YAML::Node const& node,
                                std::filesystem::path const& path) const {

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

  void OutputManager::writeTopLevelConfig() const {

    YAML::Node config;

    // some basic info
    config["name"] = name_;               // the simulation name
    config["creator"] = "CORSIKA8";       // a tag to identify C8 libraries
    config["version"] = "8.0.0-prealpha"; // the current version
    // TODO: Add current datetime as a string to the config.

    // write the node to a file
    writeNode(config, root_ / ("config.yaml"));
  }

  void OutputManager::initOutput(std::string const& name) const {
    // construct the path to this directory
    auto const path{root_ / name};

    // create the directory for this process.
    std::filesystem::create_directory(path);

    // get the config for this output
    auto config = outputs_.at(name).get().getConfig();

    // and assign the name for this output
    config["name"] = name;

    // write the config for this output to the file
    writeNode(config, path / "config.yaml");
  }

  OutputManager::OutputManager(
      std::string const& name,
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
    writeTopLevelConfig();
  }

  OutputManager::~OutputManager() {

    // if we are being destructed but EndOfRun() has not been called,
    // make sure that we gracefully close all the outputs
    if (state_ == OutputState::EventInProgress || state_ == OutputState::RunInitialized) {
      endOfRun();
    }
  }

  // void OutputManager::add(std::string const& name, BaseOutput& output) {
  template <typename TOutput>
  void OutputManager::add(std::string const& name, TOutput& output) {

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
    initOutput(name);
  }

  void OutputManager::startOfRun() {

    // this is only valid when we haven't started a run
    // or have already finished a run
    if
      (!(state_ == OutputState::RunNoInit || state_ == OutputState::RunFinished)) {

        throw std::runtime_error("startOfRun() called in invalid state.");
      }

    // we now forward this signal to all of our outputs
    for (auto& [name, output] : outputs_) {

      // construct the path to this output subdirectory
      auto const path{root_ / name};

      // and start the run
      output.get().startOfRun(path);
    }

    // we have now started running
    state_ = OutputState::RunInitialized;
  }

  void OutputManager::startOfEvent() {

    // if this is called but we are still in the initialized state,
    // make sure that we transition to EventInProgress
    // if (state_ == OutputState::RunNoInit) { startOfRun(); }

    // now start the event for all the outputs
    for (auto& [name, output] : outputs_) { output.get().startOfEvent(); }

    // and transition to the in progress state
    state_ = OutputState::EventInProgress;
  }

  void OutputManager::endOfEvent() {

    for (auto& [name, output] : outputs_) { output.get().endOfEvent(); }

    // switch back to the initialized state
    state_ = OutputState::RunInitialized;
  }

  void OutputManager::endOfRun() {

    // we can only call endOfRun when we have already started
    if (state_ == OutputState::RunNoInit) {
      throw std::runtime_error("endOfRun() called in invalid state.");
    }

    for (auto& [name, output] : outputs_) { output.get().endOfRun(); }

    // and the run has finished
    state_ = OutputState::RunFinished;

    // write any final state information into the config files
    // for (auto& [name, output] : outputs_) { output.get().endOfRun(); }
  }

} // namespace corsika
