/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/modules/proposal/InteractionModel.hpp>
#include <corsika/modules/proposal/ContinuousProcess.hpp>

namespace corsika::proposal {

  template <typename THadronicModel>
  class Interaction : public InteractionModel<THadronicModel>,
                      public InteractionProcess<Interaction<THadronicModel>> {
  public:
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, THadronicModel& model)
        : InteractionModel<THadronicModel>(env, model) {}
  };
} // namespace corsika::proposal
