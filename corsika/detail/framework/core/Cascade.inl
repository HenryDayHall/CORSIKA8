/*
 * (c) Copyright 2020 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/framework/core/PhysicalUnits.hpp>
#include <corsika/framework/process/ProcessReturn.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/media/Environment.hpp>

#include <corsika/setup/SetupStack.hpp>
#include <corsika/setup/SetupTrajectory.hpp>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

namespace corsika {

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::run() {
    setNodes(); // put each particle on stack in correct environment volume

    while (!stack_.isEmpty()) {
      while (!stack_.isEmpty()) {
        CORSIKA_LOG_TRACE("Stack: {}", stack_.asString());
        count_++;

        auto pNext = stack_.getNextParticle();

        CORSIKA_LOG_TRACE(
            "============== next particle : count={}, pid={}, "
            ", stack entries={}"
            ", stack deleted={}",
            count_, pNext.getPID(), stack_.getEntries(), stack_.getErased());

        step(pNext);
        sequence_.doStack(stack_);
      }
      // do cascade equations, which can put new particles on Stack,
      // thus, the double loop
      // doCascadeEquations();
    }
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::forceInteraction() {
    CORSIKA_LOG_TRACE("forced interaction!");
    setNodes();
    auto vParticle = stack_.getNextParticle();
    TStackView secondaries(vParticle);
    interaction(secondaries);
    sequence_.doSecondaries(secondaries);
    vParticle.erase(); // primary particle is done
  }

  template <typename TTracking, typename TProcessList, typename TStack,
            typename TStackView>
  void Cascade<TTracking, TProcessList, TStack, TStackView>::step(Particle& vParticle) {

    // determine combined total interaction length (inverse)
    InverseGrammageType const total_inv_lambda =
        sequence_.getInverseInteractionLength(vParticle);

    // sample random exponential step length in grammage
    ExponentialDistribution expDist(1 / total_inv_lambda);
    GrammageType const next_interact = expDist(rng_);

    CORSIKA_LOG_DEBUG(
        "total_lambda={} g/cm2, "
        ", next_interact={} g/cm2",
        double((1. / total_inv_lambda) / 1_g * 1_cm * 1_cm),
        double(next_interact / 1_g * 1_cm * 1_cm));

    auto const* currentLogicalNode = vParticle.getNode();

    // assert that particle stays outside void Universe if it has no
    // model properties set
    assert((currentLogicalNode != &*environment_.getUniverse() ||
            environment_.getUniverse()->hasModelProperties()) &&
           "FATAL: The environment model has no valid properties set!");

    // determine combined total inverse decay time
    InverseTimeType const total_inv_lifetime = sequence_.getInverseLifetime(vParticle);

    // sample random exponential decay time
    ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
    TimeType const next_decay = expDistDecay(rng_);

    CORSIKA_LOG_DEBUG(
        "total_lifetime={} s"
        ", next_decay={} s",
        (1 / total_inv_lifetime) / 1_s, next_decay / 1_s);

    // convert next_decay from time to length [m]
    LengthType const distance_decay = next_decay * vParticle.getMomentum().getNorm() /
                                      vParticle.getEnergy() * constants::c;

    // determine geometric tracking
    auto [step, nextVol] = tracking_.getTrack(vParticle);
    auto geomMaxLength = step.getLength(1);

    // convert next_step from grammage to length
    LengthType const distance_interact =
        currentLogicalNode->getModelProperties().getArclengthFromGrammage(step,
                                                                          next_interact);

    // determine the maximum geometric step length
    LengthType const continuous_max_dist =
        process_sequence_.getMaxStepLength(vParticle, step);

    // take minimum of geometry, interaction, decay for next step
    auto const min_distance =
        std::min({distance_interact, distance_decay, continuous_max_dist, geomMaxLength});

    CORSIKA_LOG_DEBUG(
        "transport particle by : {} m "
        "Medium transition after: {} m "
        "Decay after: {} m "
        "Interaction after: {} m "
        "Continuous limit: {} m ",
        min_distance / 1_m, geomMaxLength / 1_m, distance_decay / 1_m,
        distance_interact / 1_m, continuous_max_dist / 1_m);

    // here the particle is actually moved along the trajectory to new position:
    // std::visit(setup::ParticleUpdate<particle_type>{vParticle}, step);
    step.setLength(min_distance);
    vParticle.setPosition(step.getPosition(1));
    // assumption: tracking does not change absolute momentum:
    vParticle.setMomentum(step.getDirection(1) * vParticle.getMomentum().norm());
    vParticle.setTime(vParticle.getTime() + step.getDuration());

    // apply all continuous processes on particle + track
    if (sequence_.doContinuous(vParticle, step) == ProcessReturn::ParticleAbsorbed) {
      CORSIKA_LOG_DEBUG("Cascade: delete absorbed particle PID={} E={} GeV",
                        vParticle.getPID(), vParticle.getEnergy() / 1_GeV);
      if (!vParticle.isErased()) vParticle.erase();
      return;
    }

    CORSIKA_LOG_DEBUG("sth. happening before geometric limit ? {}",
                      ((min_distance < geomMaxLength) ? "yes" : "no"));

    if (min_distance < geomMaxLength) { // interaction to happen within geometric limit

      // check whether decay or interaction limits this step the
      // outcome of decay or interaction MAY be a) new particles in
      // secondaries, b) the projectile particle deleted (or
      // changed)

      TStackView secondaries(vParticle);

      if (min_distance < continuous_max_dist) {
        /*
          Create SecondaryView object on Stack. The data container
          remains untouched and identical, and 'projectil' is identical
          to 'vParticle' above this line. However,
          projectil.AddSecondaries populate the SecondaryView, which can
          then be used afterwards for further processing. Thus: it is
          important to use projectle/view (and not vParticle) for Interaction,
          and Decay!
        */

        [[maybe_unused]] auto projectile = secondaries.getProjectile();

        if (distance_interat < distance_decay) {
          interaction(secondaries);
        } else {
          decay(secondaries);
          // make sure particle actually did decay if it should have done so
          if (secondaries.getSize() == 1 &&
              projectile.getPID() == secondaries.getNextParticle().getPID())
            throw std::runtime_error(fmt::format("Particle {} decays into itself!",
                                                 get_name(projectile.getPID())));
        }

        sequence_.doSecondaries(secondaries);
        vParticle.erase();
      } else { // step-length limitation within volume

        CORSIKA_LOG_DEBUG("step-length limitation");
        // no further physics happens here. just proceed to next step.
      }

      [[maybe_unused]] auto const assertion = [&] {
        auto const* numericalNodeAfterStep =
            environment_.getUniverse()->getContainingNode(vParticle.getPosition());
        CORSIKA_LOG_TRACE(
            "Geometry check: numericalNodeAfterStep={} currentLogicalNode={}",
            fmt::ptr(numericalNodeAfterStep), fmt::ptr(currentLogicalNode));
        return numericalNodeAfterStep == currentLogicalNode;
      };

      assert(assertion()); // numerical and logical nodes should match, since
                           // we did not cross any volume boundary

    } else { // boundary crossing, step is limited by volume boundary

      if (nextVol != currentLogicalNode) {

        CORSIKA_LOG_DEBUG("volume boundary crossing to {}", fmt::ptr(nextVol));

        if (nextVol == environment_.getUniverse().get()) {
          CORSIKA_LOG_DEBUG(
              "particle left physics world, is now in unknown space -> delete");
          vParticle.erase();
        }
        vParticle.setNode(nextVol);
        /*
          doBoundary may delete the particle (or not)

          caveat: any changes to vParticle, or even the production
          of new secondaries is currently not passed to ParticleCut,
          thus, particles outside the desired phase space may be produced.

          \todo: this must be fixed.
        */

        sequence_.doBoundaryCrossing(vParticle, *currentLogicalNode, *nextVol);
      }
    }

    template <typename TTracking, typename TProcessList, typename TStack,
              typename TStackView>
    ProcessReturn Cascade<TTracking, TProcessList, TStack, TStackView>::decay(TStackView &
                                                                              view) {
      CORSIKA_LOG_DEBUG("decay");
      InverseTimeType const actual_decay_time =
          sequence_.getInverseLifetime(view.parent());

      UniformRealDistribution<InverseTimeType> uniDist(actual_decay_time);
      const auto sample_process = uniDist(rng_);

      auto const returnCode = sequence_.selectDecay(view, sample_process);
      if (returnCode != ProcessReturn::Decayed) {
        CORSIKA_LOG_WARN("Particle did not decay!");
      }
      setEventType(view, history::EventType::Decay);
      return returnCode;
    }

    template <typename TTracking, typename TProcessList, typename TStack,
              typename TStackView>
    ProcessReturn Cascade<TTracking, TProcessList, TStack, TStackView>::interaction(
        TStackView & view) {
      CORSIKA_LOG_DEBUG("collide");

      InverseGrammageType const current_inv_length =
          sequence_.getInverseInteractionLength(view.parent());

      UniformRealDistribution<InverseGrammageType> uniDist(current_inv_length);

      const auto sample_process = uniDist(rng_);
      auto const returnCode = sequence_.selectInteraction(view, sample_process);
      if (returnCode != ProcessReturn::Interacted) {
        CORSIKA_LOG_WARN("Particle did not interace!");
      }
      setEventType(view, history::EventType::Interaction);
      return returnCode;
    }

    template <typename TTracking, typename TProcessList, typename TStack,
              typename TStackView>
    void Cascade<TTracking, TProcessList, TStack, TStackView>::setNodes() {
      std::for_each(stack_.begin(), stack_.end(), [&](auto& p) {
        auto const* numericalNode =
            environment_.getUniverse()->getContainingNode(p.getPosition());
        p.setNode(numericalNode);
      });
    }

    template <typename TTracking, typename TProcessList, typename TStack,
              typename TStackView>
    void Cascade<TTracking, TProcessList, TStack, TStackView>::setEventType(
        TStackView & view, [[maybe_unused]] history::EventType eventType) {
      if constexpr (TStackView::has_event) {
        for (auto&& sec : view) { sec.getEvent()->setEventType(eventType); }
      }
    }

  } // namespace corsika
