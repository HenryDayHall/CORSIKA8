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

#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

namespace corsika {

  std::string OutputManager::getCurrentTime() const {

    // the format for our date string
    auto fmt{"%d/%m/%Y %H:%M:%S %Z"};

    // get the current time
    auto t = std::time(nullptr);
    auto current = *std::localtime(&t);

    // create the string and push the time onto it
    std::ostringstream oss;
    oss << std::put_time(&current, fmt);

    return oss.str();
  }

  void OutputManager::writeNode(YAML::Node const& node,
                                std::filesystem::path const& path) const {

    // construct a YAML emitter for this config file
    YAML::Emitter out;

    // and write the node to the output
    out << node;

    // open the output file - this is <output name>.yaml
    std::ofstream file(path.string());

    // dump the YAML to the file
    file << out.c_str() << std::endl;
  }

  void OutputManager::writeTopLevelConfig() const {

    YAML::Node config;

    // some basic info
    config["name"] = name_;               // the simulation name
    config["creator"] = "CORSIKA8";       // a tag to identify C8 libraries
    config["version"] = "8.0.0-prealpha"; // the current version
    config["start time"] = getCurrentTime();

    // write the node to a file
    writeNode(config, root_ / ("config.yaml"));
  }

  // void OutputManager::writeTopLevelSummary() const {

  //   YAML::Node config;

  //   // some basic info
  //   config["start time"] = getCurrentTime(); // TODO:
  //   config["end time"] = getCurrentTime();
  //   // config["showers"] = 0; // TODO

  //   // write the node to a file
  //   writeNode(config, root_ / ("summary.yaml"));
  // }

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

    // construct the directory for this library
    std::filesystem::create_directory(root_);

    // write the top level config file
    writeTopLevelConfig();
  }

  OutputManager::~OutputManager() {

    if (state_ == OutputState::ShowerInProgress) {
      // if this the destructor is called before the shower has been explicitly
      // ended, print a warning and end the shower before continuing.
      logger->warn(
          "OutputManager was destroyed before endOfShower() called."
          " The last shower in this libray may be incomplete.");
      endOfShower();
    }

    // if we are being destructed but EndOfLibrary() has not been called,
    // make sure that we gracefully close all the outputs. This is a supported
    // method of operation so we don't issue a warning here
    if (state_ == OutputState::LibraryReady) { endOfLibrary(); }
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

  void OutputManager::startOfLibrary() {

    // this is only valid when we haven't started a library
    // or have already finished a library
    if (!(state_ == OutputState::NoInit || state_ == OutputState::LibraryFinished)) {

      throw std::runtime_error("startOfLibrary() called in invalid state.");
    }

    // we now forward this signal to all of our outputs
    for (auto& [name, output] : outputs_) {

      // construct the path to this output subdirectory
      auto const path{root_ / name};

      // and start the library
      output.get().startOfLibrary(path);
    }

    // we have now started running
    state_ = OutputState::LibraryReady;
  }

  void OutputManager::startOfShower() {

    // if this is called and we still in the "no init" state, then
    // this is the first shower in the library so make sure we start it
    if (state_ == OutputState::NoInit) { startOfLibrary(); }

    // now start the event for all the outputs
    for (auto& [name, output] : outputs_) { output.get().startOfShower(); }

    // and transition to the in progress state
    state_ = OutputState::ShowerInProgress;
  }

  void OutputManager::endOfShower() {

    for (auto& [name, output] : outputs_) { output.get().endOfShower(); }

    // switch back to the initialized state
    state_ = OutputState::LibraryReady;
  }

  void OutputManager::endOfLibrary() {

    // we can only call endOfLibrary when we have already started
    if (state_ == OutputState::NoInit) {
      throw std::runtime_error("endOfLibrary() called in invalid state.");
    }

    // forward the endOfLibrary() call to all the registered outputs
    for (auto& [name, output] : outputs_) { output.get().endOfLibrary(); }

    // and the library has finished
    state_ = OutputState::LibraryFinished;
  }

} // namespace corsika
