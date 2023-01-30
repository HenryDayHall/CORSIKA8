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

  template <typename THadronicLEModel, typename THadronicHEModel>
  class Interaction
      : public InteractionModel<THadronicLEModel, THadronicHEModel>,
        public InteractionProcess<Interaction<THadronicLEModel, THadronicHEModel>> {
  public:
    template <typename TEnvironment>
    Interaction(TEnvironment const& env, THadronicLEModel& modelLE,
                THadronicHEModel& modelHE, HEPEnergyType const& thr)
        : InteractionModel<THadronicLEModel, THadronicHEModel>(env, modelLE, modelHE,
                                                               thr) {}
  };
} // namespace corsika::proposal
