/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */
#pragma once

#include <corsika/modules/radio/propagators/RadioPropagator.hpp>

namespace corsika {

  template <typename TImpl, typename TEnvironment>
  inline RadioPropagator<TImpl, TEnvironment>::RadioPropagator(TEnvironment const& env)
      : env_(env) {}

} // namespace corsika
