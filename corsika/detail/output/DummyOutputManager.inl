/*
 * (c) Copyright 2021 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

namespace corsika {

  DummyOutputManager::DummyOutputManager() {}

  DummyOutputManager::~DummyOutputManager() {}

  template <typename TOutput>
  void DummyOutputManager::add(std::string const& name, TOutput& output) {}

  void DummyOutputManager::startOfLibrary() {}

  void DummyOutputManager::startOfShower() {}

  void DummyOutputManager::endOfShower() {}

  void DummyOutputManager::endOfLibrary() {}

} // namespace corsika
