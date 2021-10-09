/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/sibyll/ParticleConversion.hpp>
#include <corsika/modules/sibyll/InteractionModel.hpp>
#include <corsika/modules/sibyll/Decay.hpp>
#include <corsika/modules/sibyll/NuclearInteractionModel.hpp>

#include <corsika/framework/process/InteractionProcess.hpp>

/**
 * @file Sibyll.hpp
 *
 * Includes all the parts of the Sibyll model. Defines the InteractoinProcess<TModel>
 * classes needed for the ProcessSequence.
 */

namespace corsika::sibyll {
  /**
   * @brief sibyll::Interaction is the process for ProcessSequence.
   *
   * The sibyll::Model is wrapped as an InteractionProcess here in order
   * to provide all the functions for ProcessSequence.
   */
  class Interaction : public InteractionModel, public InteractionProcess<Interaction> {};

  /**
   * @brief sibyll::NuclearInteraction is the process for ProcessSequence.
   *
   * The sibyll::NuclearInteractionModel is wrapped as an InteractionProcess here in order
   * to provide all the functions for ProcessSequence.
   */
  template <class TEnvironment, class TNucleonModel>
  class NuclearInteraction
      : public NuclearInteractionModel<TEnvironment, TNucleonModel>,
        public InteractionProcess<NuclearInteraction<TEnvironment, TNucleonModel>> {
  public:
    NuclearInteraction(TNucleonModel& model, TEnvironment const& env)
        : NuclearInteractionModel<TNucleonModel, TEnvironment>(model, env) {}
  };

} // namespace corsika::sibyll
