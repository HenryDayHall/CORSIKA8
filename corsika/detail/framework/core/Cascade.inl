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
#include <corsika/framework/process/ContinuousProcessStepLength.hpp>
#include <corsika/framework/process/ContinuousProcessIndex.hpp>
#include <corsika/framework/random/ExponentialDistribution.hpp>
#include <corsika/framework/random/RNGManager.hpp>
#include <corsika/framework/random/UniformRealDistribution.hpp>
#include <corsika/framework/stack/SecondaryView.hpp>
#include <corsika/media/Environment.hpp>

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

namespace corsika {

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::run() {
    setNodes(); // put each particle on stack in correct environment volume

    while (!stack_.isEmpty()) {

      sequence_.initCascadeEquations();

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
      sequence_.doCascadeEquations(stack_);
    }
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::forceInteraction() {
    CORSIKA_LOG_TRACE("forced interaction!");
    setNodes();
    auto vParticle = stack_.getNextParticle();
    stack_view_type secondaries(vParticle);
    interaction(secondaries, sequence_.getInverseInteractionLength(vParticle));
    sequence_.doSecondaries(secondaries);
    vParticle.erase(); // primary particle is done
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::step(
      particle_type& vParticle) {

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
    ContinuousProcessStepLength const continuousMaxStep =
        sequence_.getMaxStepLength(vParticle, step);
    LengthType const continuous_max_dist = continuousMaxStep;

    // take minimum of geometry, interaction, decay for next step
    LengthType const min_discrete = std::min(distance_interact, distance_decay);
    LengthType const min_non_continuous = std::min(min_discrete, geomMaxLength);
    LengthType const min_distance = std::min(min_non_continuous, continuous_max_dist);

    // inform ContinuousProcesses (if applicable) that it is responsible for step-limit
    // this would become simpler if we follow the idea of Max to enumerate ALL types of
    // processes. Then non-continuous are included and no further logic is needed to
    // distinguish between continuous and non-continuous limit.
    ContinuousProcessIndex limitingId;
    bool const isContinuous = continuous_max_dist < min_non_continuous;
    if (isContinuous) {
      limitingId =
          continuousMaxStep; // the current step IS limited by a known continuous process
    }

    CORSIKA_LOG_DEBUG(
        "transport particle by : {} m "
        "Medium transition after: {} m "
        "Decay after: {} m "
        "Interaction after: {} m "
        "Continuous limit: {} m ",
        min_distance / 1_m, geomMaxLength / 1_m, distance_decay / 1_m,
        distance_interact / 1_m, continuous_max_dist / 1_m);

    // move particle along the trajectory to new position
    // also update momentum/direction/time
    step.setLength(min_distance);
    vParticle.setPosition(step.getPosition(1));
    // assumption: tracking does not change absolute momentum (continuous physics can and
    // will):
    vParticle.setMomentum(step.getDirection(1) * vParticle.getMomentum().getNorm());

    // apply all continuous processes on particle + track
    if (sequence_.doContinuous(vParticle, step, limitingId) ==
        ProcessReturn::ParticleAbsorbed) {
      CORSIKA_LOG_DEBUG("Cascade: delete absorbed particle PID={} E={} GeV",
                        vParticle.getPID(), vParticle.getEnergy() / 1_GeV);
      if (vParticle.isErased()) {
        CORSIKA_LOG_WARN(
            "Particle marked as Absorbed in doContinuous, but prematurely erased. This "
            "may be bug. Check.");
      } else {
        vParticle.erase();
      }
      return;
    }
    vParticle.setTime(vParticle.getTime() + step.getDuration());
    if (isContinuous) {
      return; // there is nothing further, step is finished
    }

    CORSIKA_LOG_DEBUG("discrete process before geometric limit ? {}",
                      ((min_distance < geomMaxLength) ? "yes" : "no"));

    if (geomMaxLength < min_discrete) {
      // geometric / tracking limit

      if (nextVol != currentLogicalNode) {
        // boundary crossing, step is limited by volume boundary

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
        return; // step finished
      }

      CORSIKA_LOG_DEBUG("step limit reached (e.g. deflection). nothing further happens.");

      {
        auto const* numericalNodeAfterStep =
            environment_.getUniverse()->getContainingNode(vParticle.getPosition());
        CORSIKA_LOG_TRACE(
            "Geometry check: numericalNodeAfterStep={} currentLogicalNode={}",
            fmt::ptr(numericalNodeAfterStep), fmt::ptr(currentLogicalNode));
        if (numericalNodeAfterStep != currentLogicalNode) {
          CORSIKA_LOG_ERROR(
              "expect to be in node currentLogicalNode={} but are in "
              "numericalNodeAfterStep={}. Continue, but without guarantee.",
              fmt::ptr(currentLogicalNode), fmt::ptr(numericalNodeAfterStep));
        }
      }
      // we did not cross any volume boundary

      // step length limit
      return;
    }

    // interaction or decay to happen in this step
    // the outcome of decay or interaction MAY be a) new particles in
    // secondaries, b) the projectile particle deleted (or
    // changed)

    stack_view_type secondaries(vParticle);

    /*
      Create SecondaryView object on Stack. The data container
      remains untouched and identical, and 'projectile' is identical
      to 'vParticle' above this line. However,
      projectile.AddSecondaries populate the SecondaryView, which can
      then be used afterwards for further processing. Thus: it is
      important to use projectile/view (and not vParticle) for Interaction,
      and Decay!
    */

    [[maybe_unused]] auto projectile = secondaries.getProjectile();

    if (distance_interact < distance_decay) {
      interaction(secondaries, total_inv_lambda);
    } else {
      if (decay(secondaries, total_inv_lifetime) == ProcessReturn::Decayed) {
        if (secondaries.getSize() == 1 &&
            projectile.getPID() == secondaries.getNextParticle().getPID()) {
          throw std::runtime_error(fmt::format("Particle {} decays into itself!",
                                               get_name(projectile.getPID())));
        }
      }
    }

    sequence_.doSecondaries(secondaries);
    vParticle.erase();
  } // namespace corsika

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline ProcessReturn Cascade<TTracking, TProcessList, TOutput, TStack>::decay(
      stack_view_type& view, InverseTimeType initial_inv_decay_time) {
    CORSIKA_LOG_DEBUG("decay");

#ifdef DEBUG
    InverseTimeType const actual_decay_time = sequence_.getInverseLifetime(view.parent());
    if (actual_decay_time * 0.99 > initial_inv_decay_time) {
      CORSIKA_LOG_WARN(
          "Decay time decreased during step! This leads to un-physical step length. "
          "delta_inverse_decay_time={}",
          (actual_decay_time != InverseTimeType::zero() &&
                   initial_inv_decay_time != InverseTimeType::zero()
               ? 1 / initial_inv_decay_time - 1 / actual_decay_time
               : TimeType::zero()));
    }
#endif

    // one option is that decay_time is now larger (less
    // probability for decay) than it was before the step, thus,
    // no decay might actually occur and is allowed

    UniformRealDistribution<InverseTimeType> uniDist(initial_inv_decay_time);
    const auto sample_process = uniDist(rng_);

    auto const returnCode = sequence_.selectDecay(view, sample_process);
    if (returnCode != ProcessReturn::Decayed) {
      CORSIKA_LOG_DEBUG("Particle did not decay!");
    }
    setEventType(view, history::EventType::Decay);
    return returnCode;
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline ProcessReturn Cascade<TTracking, TProcessList, TOutput, TStack>::interaction(
      stack_view_type& view, InverseGrammageType initial_inv_int_length) {
    CORSIKA_LOG_DEBUG("collide");

#ifdef DEBUG
    InverseGrammageType const actual_inv_length = sequence_.getInverseInteractionLength(
        view.parent()); // 1/lambda_int after step, -dE/dX etc.

    if (actual_inv_length * 0.99 > initial_inv_int_length) {
      CORSIKA_LOG_WARN(
          "Interaction length decreased during step! This leads to un-physical step "
          "length. delta_inverse_interaction_length={}",
          1 / initial_inv_int_length - 1 / actual_inv_length);
    }
#endif

    // one option is that interaction_length is now larger (less
    // probability for collision) than it was before the step, thus,
    // no interaction might actually occur and is allowed

    UniformRealDistribution<InverseGrammageType> uniDist(initial_inv_int_length);
    const auto sample_process = uniDist(rng_);
    auto const returnCode = sequence_.selectInteraction(view, sample_process);
    if (returnCode != ProcessReturn::Interacted) {
      CORSIKA_LOG_DEBUG("Particle did not interact!");
    }
    setEventType(view, history::EventType::Interaction);
    return returnCode;
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::setNodes() {
    std::for_each(stack_.begin(), stack_.end(), [&](auto& p) {
      auto const* numericalNode =
          environment_.getUniverse()->getContainingNode(p.getPosition());
      p.setNode(numericalNode);
    });
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::setEventType(
      stack_view_type& view, [[maybe_unused]] history::EventType eventType) {
    if constexpr (stack_view_type::has_event) {
      for (auto&& sec : view) { sec.getEvent()->setEventType(eventType); }
    }
  }

} // namespace corsika
