/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/fluka/ParticleConversion.hpp>
#include <corsika/modules/fluka/InteractionModel.hpp>
#include <corsika/framework/process/InteractionProcess.hpp>

/**
 * @file FLUKA.hpp
 *
 * Includes all the parts of the FLUKA model. Defines the InteractionProcess<TModel>
 * classes needed for the ProcessSequence.
 */

namespace corsika::fluka {
  /**
   * fluka::Interaction is the process for ProcessSequence.
   *
   * The fluka::InteractionModel is wrapped as an InteractionProcess here in order
   * to provide all the functions for ProcessSequence.
   */
  class Interaction : public fluka::InteractionModel,
                      public corsika::InteractionProcess<Interaction> {};
} // namespace corsika::fluka
