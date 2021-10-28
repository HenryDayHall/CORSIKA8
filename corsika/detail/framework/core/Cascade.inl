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

#include <corsika/framework/utility/COMBoost.hpp>

#include <corsika/media/Environment.hpp>
#include <corsika/media/NuclearComposition.hpp>

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
            "============== next particle : count={}, pid={}"
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
    auto particle = stack_.getNextParticle();
    stack_view_type secondaries(particle);

    auto const* currentLogicalNode = particle.getNode();
    // assert that particle stays outside void Universe if it has no
    // model properties set
    assert((currentLogicalNode != &*environment_.getUniverse() ||
            environment_.getUniverse()->hasModelProperties()) &&
           "FATAL: The environment model has no valid properties set!");
    NuclearComposition const& composition =
        currentLogicalNode->getModelProperties().getNuclearComposition();

    // determine projectile
    HEPEnergyType const Elab = particle.getEnergy();
    FourMomentum const projectileP4{Elab, particle.getMomentum()};
    // determine cross section in material
    CrossSectionType const sigma =
        composition.getWeightedSum([=](Code const targetId) -> CrossSectionType {
          FourMomentum const targetP4(
              get_mass(targetId),
              MomentumVector(particle.getMomentum().getCoordinateSystem(),
                             {0_GeV, 0_GeV, 0_GeV}));
          return sequence_.getCrossSection(particle, targetId, targetP4);
        });
    interaction(secondaries, projectileP4, composition, sigma);
    sequence_.doSecondaries(secondaries);
    particle.erase(); // primary particle is done
  }

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline void Cascade<TTracking, TProcessList, TOutput, TStack>::step(
      particle_type& particle) {

    // determine the volume where the particle is (last) known to be
    auto const* currentLogicalNode = particle.getNode();

    // assert that particle stays outside void Universe if it has no
    // model properties set
    assert((currentLogicalNode != &*environment_.getUniverse() ||
            environment_.getUniverse()->hasModelProperties()) &&
           "FATAL: The environment model has no valid properties set!");

    NuclearComposition const& composition =
        currentLogicalNode->getModelProperties().getNuclearComposition();

    // determine projectile
    HEPEnergyType const Elab = particle.getEnergy();
    FourMomentum const projectileP4{Elab, particle.getMomentum()};

    // determine combined full inelastic cross section of the particles in the material

    CrossSectionType const total_cx =
        composition.getWeightedSum([=](Code const targetId) -> CrossSectionType {
          FourMomentum const targetP4(
              get_mass(targetId),
              MomentumVector(particle.getMomentum().getCoordinateSystem(),
                             {0_GeV, 0_GeV, 0_GeV}));
          return sequence_.getCrossSection(particle, targetId, targetP4);
        });

    // calculate interaction length in medium
    GrammageType const total_lambda =
        (composition.getAverageMassNumber() * constants::u) / total_cx;

    // sample random exponential step length in grammage
    ExponentialDistribution expDist(total_lambda);
    GrammageType const next_interact = expDist(rng_);

    CORSIKA_LOG_DEBUG("total_lambda={} g/cm2, next_interact={} g/cm2",
                      double(total_lambda / 1_g * 1_cm * 1_cm),
                      double(next_interact / 1_g * 1_cm * 1_cm));

    // determine combined total inverse decay time
    InverseTimeType const total_inv_lifetime = sequence_.getInverseLifetime(particle);

    // sample random exponential decay time
    ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
    TimeType const next_decay = expDistDecay(rng_);

    CORSIKA_LOG_DEBUG("total_lifetime={} ns, next_decay={} ns",
                      (1 / total_inv_lifetime) / 1_ns, next_decay / 1_ns);

    // convert next_decay from time to length [m]
    LengthType const distance_decay = next_decay * particle.getMomentum().getNorm() /
                                      particle.getEnergy() * constants::c;

    // determine geometric tracking
    auto [step, nextVol] = tracking_.getTrack(particle);
    auto geomMaxLength = step.getLength(1);

    // convert next_step from grammage to length
    LengthType const distance_interact =
        currentLogicalNode->getModelProperties().getArclengthFromGrammage(step,
                                                                          next_interact);

    // determine the maximum geometric step length
    ContinuousProcessStepLength const continuousMaxStep =
        sequence_.getMaxStepLength(particle, step);
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

    // apply all continuous processes on particle + track
    if (sequence_.doContinuous(particle, step, limitingId) ==
        ProcessReturn::ParticleAbsorbed) {
      CORSIKA_LOG_DEBUG("Cascade: delete absorbed particle PID={} E={} GeV",
                        particle.getPID(), particle.getEnergy() / 1_GeV);
      if (particle.isErased()) {
        CORSIKA_LOG_WARN(
            "Particle marked as Absorbed in doContinuous, but prematurely erased. This "
            "may be bug. Check.");
      } else {
        particle.erase();
      }
      return; // particle is gone -> return
    }
    particle.setTime(particle.getTime() + step.getDuration());
    particle.setPosition(step.getPosition(1));
    particle.setMomentum(step.getDirection(1) * particle.getMomentum().getNorm());

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
          particle.erase();
        }
        particle.setNode(nextVol);
        /*
          doBoundary may delete the particle (or not)

          caveat: any changes to particle, or even the production
          of new secondaries is currently not passed to ParticleCut,
          thus, particles outside the desired phase space may be produced.

          \todo: this must be fixed.
        */

        sequence_.doBoundaryCrossing(particle, *currentLogicalNode, *nextVol);
        return; // step finished
      }

      CORSIKA_LOG_DEBUG("step limit reached (e.g. deflection). nothing further happens.");

      // final sanity check, no actions
      {
        auto const* numericalNodeAfterStep =
            environment_.getUniverse()->getContainingNode(particle.getPosition());
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

    stack_view_type secondaries(particle);

    /*
      Create SecondaryView object on Stack. The data container
      remains untouched and identical, and 'projectile' is identical
      to 'particle' above this line. However,
      projectile.addSecondaries populate the SecondaryView, which can
      then be used afterwards for further processing. Thus: it is
      important to use projectile/view (and not particle) for Interaction,
      and Decay!
    */
    if (distance_interact < distance_decay) {
      interaction(secondaries, projectileP4, composition, total_cx);
    } else {
      [[maybe_unused]] auto projectile = secondaries.getProjectile();

      if (decay(secondaries, total_inv_lifetime) == ProcessReturn::Decayed) {
        if (secondaries.getSize() == 1 &&
            projectile.getPID() == secondaries.getNextParticle().getPID()) {
          throw std::runtime_error(fmt::format("Particle {} decays into itself!",
                                               get_name(projectile.getPID())));
        }
      }
    }

    sequence_.doSecondaries(secondaries);
    particle.erase();
  } // namespace corsika

  template <typename TTracking, typename TProcessList, typename TOutput, typename TStack>
  inline ProcessReturn Cascade<TTracking, TProcessList, TOutput, TStack>::decay(
      stack_view_type& view, InverseTimeType initial_inv_decay_time) {
    CORSIKA_LOG_DEBUG("decay");

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
      stack_view_type& view, FourMomentum const& projectileP4,
      NuclearComposition const& composition,
      CrossSectionType const initial_cross_section) {

    CORSIKA_LOG_DEBUG("collide");

    // one option is that cross section is now smaller (less
    // probability for collision) than it was before the step, thus,
    // no interaction might actually occur and is allowed

    UniformRealDistribution<CrossSectionType> uniDist(initial_cross_section);
    CrossSectionType const sample_process_by_cx = uniDist(rng_);
    auto const returnCode = sequence_.selectInteraction(view, projectileP4, composition,
                                                        rng_, sample_process_by_cx);
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
