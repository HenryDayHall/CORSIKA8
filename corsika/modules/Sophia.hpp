/*
 * (c) Copyright 2022 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sophia/ParticleConversion.hpp>

#include <corsika/modules/sophia/InteractionModel.hpp>

#include <corsika/framework/process/InteractionProcess.hpp>

/**
 * @file Sophia.hpp
 *
 * Includes all the parts of the Sophia model. Defines the InteractionProcess<TModel>
 * classes needed for the ProcessSequence.
 */

namespace corsika::sophia {
  /**
   * @brief sophia::Interaction is the process for ProcessSequence.
   *
   * The sophia::InteractionModel is wrapped as an InteractionProcess here in order
   * to provide all the functions for ProcessSequence.
   */
  // struct Interaction : public InteractionModel, public InteractionProcess<Interaction> {
  //   template <typename TEnvironment>
  //   Interaction(TEnvironment const& env)
  //       : InteractionModel{env} {}
  // };

} // namespace corsika::sophia
