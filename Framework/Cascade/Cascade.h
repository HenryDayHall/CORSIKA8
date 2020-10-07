/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#pragma once

#include <corsika/environment/Environment.h>
#include <corsika/logging/Logging.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/stack/SecondaryView.h>
#include <corsika/units/PhysicalUnits.h>
#include <corsika/stack/history/EventType.hpp>
#include <corsika/stack/history/HistorySecondaryProducer.hpp>

#include <corsika/setup/SetupTrajectory.h>

/*  see Issue 161, we need to include SetupStack only because we need
    to globally define StackView. This is clearly not nice and should
    be changed, when possible. It might be that StackView needs to be
    templated in Cascade, but this would be even worse... so we don't
    do that until it is really needed.
 */
#include <corsika/setup/SetupStack.h>

#include <cassert>
#include <cmath>
#include <limits>

/**
 * The cascade namespace assembles all objects needed to simulate full particles cascades.
 */

namespace corsika::cascade {

  /**
   * \class Cascade
   *
   * The Cascade class is constructed from template arguments making
   * it very versatile. Via the template arguments physics models are
   * plugged into the cascade simulation.
   *
   * <b>TTracking</b> must be a class according to the
   * TrackingInterface providing the functions:
   * <code>auto GetTrack(Particle const& p)</auto>,
   * with the return type <code>geometry::Trajectory<corsika::geometry::Line>
   * </code>
   *
   * <b>TProcessList</b> must be a ProcessSequence.   *
   * <b>Stack</b> is the storage object for particle data, i.e. with
   * Particle class type <code>Stack::ParticleType</code>
   *
   *
   */

  template <typename TTracking, typename TProcessList, typename TStack,
            /*
              TStackView is needed as explicit template parameter because
              of issue 161 and the
              inability of clang to understand "stack::MakeView" so far.
             */
            typename TStackView = corsika::setup::StackView>
  class Cascade {
    using Particle = typename TStack::ParticleType;
    using VolumeTreeNode =
        std::remove_pointer_t<decltype(((Particle*)nullptr)->GetNode())>;
    using MediumInterface = typename VolumeTreeNode::IModelProperties;

  private:
    // Data members
    corsika::environment::Environment<MediumInterface> const& fEnvironment;
    TTracking& fTracking;
    TProcessList& fProcessSequence;
    TStack& fStack;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
    unsigned int count_ = 0;

  private:
    // we only want fully configured objects
    Cascade() = delete;

  public:
    /**
     * Cascade class cannot be default constructed, but needs a valid
     * list of physics processes for configuration at construct time.
     */
    Cascade(corsika::environment::Environment<MediumInterface> const& env, TTracking& tr,
            TProcessList& pl, TStack& stack)
        : fEnvironment(env)
        , fTracking(tr)
        , fProcessSequence(pl)
        , fStack(stack)
        , count_(0) {
      C8LOG_INFO(c8_ascii_);
#ifdef WITH_HISTORY
      C8LOG_INFO(" - With full cascade HISTORY.");
#endif
    }

    /**
     * set the nodes for all particles on the stack according to their numerical
     * position
     */
    void SetNodes() {
      std::for_each(fStack.begin(), fStack.end(), [&](auto& p) {
        auto const* numericalNode =
            fEnvironment.GetUniverse()->GetContainingNode(p.GetPosition());
        p.SetNode(numericalNode);
      });
    }

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void Run() {
      SetNodes();

      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          C8LOG_TRACE(fmt::format("Stack: {}", fStack.as_string()));
          count_++;
          auto pNext = fStack.GetNextParticle();
          C8LOG_DEBUG(fmt::format(
              "============== next particle : count={}, pid={}, "
              ", stack entries={}"
              ", stack deleted={}",
              count_, pNext.GetPID(), fStack.getEntries(), fStack.getDeleted()));
          Step(pNext);
          fProcessSequence.DoStack(fStack);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
    }

    /**
     * Force an interaction of the top particle of the stack at its current position.
     * Note that SetNodes() or an equivalent procedure needs to be called first if you
     * want to call forceInteraction() for the primary interaction.
     */
    void forceInteraction() {
      C8LOG_DEBUG("forced interaction!");
      auto vParticle = fStack.GetNextParticle();
      TStackView secondaries(vParticle);
      interaction(vParticle, secondaries);
      fProcessSequence.DoSecondaries(secondaries);
      vParticle.Delete(); // todo: this should be reviewed, see below
    }

  private:
    /**
     * The Step function is executed for each particle from the
     * stack. It will calcualte geometric transport of the particles,
     * and apply continuous and stochastic processes to it, which may
     * lead to energy losses, scattering, absorption, decays and the
     * production of secondary particles.
     *
     * New particles produced in one step are subject to further
     * processing, e.g. thinning, etc.
     */
    void Step(Particle& vParticle) {
      using namespace corsika;
      using namespace corsika::units::si;

      // determine geometric tracking
      auto [step, geomMaxLength, nextVol] = fTracking.GetTrack(vParticle);
      [[maybe_unused]] auto const& dummy_nextVol = nextVol;

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          fProcessSequence.GetTotalInverseInteractionLength(vParticle);

      // sample random exponential step length in grammage
      corsika::random::ExponentialDistribution expDist(1 / total_inv_lambda);
      GrammageType const next_interact = expDist(fRNG);

      C8LOG_DEBUG(
          "total_lambda={} g/cm2, "
          ", next_interact={} g/cm2",
          double((1. / total_inv_lambda) / 1_g * 1_cm * 1_cm),
          double(next_interact / 1_g * 1_cm * 1_cm));

      auto const* currentLogicalNode = vParticle.GetNode();

      // assert that particle stays outside void Universe if it has no
      // model properties set
      assert(currentLogicalNode != &*fEnvironment.GetUniverse() ||
             fEnvironment.GetUniverse()->HasModelProperties());

      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(step,
                                                                         next_interact);

      // determine the maximum geometric step length from continuous processes
      LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, step);
      C8LOG_DEBUG("distance_max={} m", distance_max / 1_m);

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          fProcessSequence.GetTotalInverseLifetime(vParticle);

      // sample random exponential decay time
      corsika::random::ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
      TimeType const next_decay = expDistDecay(fRNG);
      C8LOG_DEBUG(
          "total_lifetime={} s"
          ", next_decay={} s",
          (1 / total_inv_lifetime) / 1_s, next_decay / 1_s);

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * vParticle.GetMomentum().norm() /
                                        vParticle.GetEnergy() * units::constants::c;

      // take minimum of geometry, interaction, decay for next step
      auto const min_distance =
          std::min({distance_interact, distance_decay, distance_max, geomMaxLength});

      C8LOG_DEBUG("transport particle by : {} m", min_distance / 1_m);

      // here the particle is actually moved along the trajectory to new position:
      // std::visit(setup::ParticleUpdate<Particle>{vParticle}, step);
      vParticle.SetPosition(step.PositionFromArclength(min_distance));
      // .... also update time, momentum, direction, ...
      vParticle.SetTime(vParticle.GetTime() + min_distance / units::constants::c);

      step.LimitEndTo(min_distance);

      // apply all continuous processes on particle + track
      process::EProcessReturn status = fProcessSequence.DoContinuous(vParticle, step);

      if (status == process::EProcessReturn::eParticleAbsorbed) {
        C8LOG_DEBUG("Cascade: delete absorbed particle PID={} E={} GeV",
                    vParticle.GetPID(), vParticle.GetEnergy() / 1_GeV);
        vParticle.Delete();
        return;
      }

      C8LOG_DEBUG("sth. happening before geometric limit ? {}",
                  ((min_distance < geomMaxLength) ? "yes" : "no"));

      if (min_distance < geomMaxLength) { // interaction to happen within geometric limit

        // check whether decay or interaction limits this step the
        // outcome of decay or interaction MAY be a) new particles in
        // secondaries, b) the projectile particle deleted (or
        // changed)

        TStackView secondaries(vParticle);

        if (min_distance != distance_max) {
          /*
            Create SecondaryView object on Stack. The data container
            remains untouched and identical, and 'projectil' is identical
            to 'vParticle' above this line. However,
            projectil.AddSecondaries populate the SecondaryView, which can
            then be used afterwards for further processing. Thus: it is
            important to use projectle/view (and not vParticle) for Interaction,
            and Decay!
          */

          [[maybe_unused]] auto projectile = secondaries.GetProjectile();

          if (min_distance == distance_interact) {
            interaction(vParticle, secondaries);
          } else {
            assert(min_distance == distance_decay);
            decay(vParticle, secondaries);
            // make sure particle actually did decay if it should have done so
            if (secondaries.getSize() == 1 &&
                projectile.GetPID() == secondaries.GetNextParticle().GetPID())
              throw std::runtime_error(
                  fmt::format("Cascade: {} decayed into itself!",
                              particles::GetName(projectile.GetPID())));
          }

          fProcessSequence.DoSecondaries(secondaries);
          vParticle.Delete();

        } else { // step-length limitation within volume
          C8LOG_DEBUG("step-length limitation");
          // no extra physics happens here. just proceed to next step.
        }

        [[maybe_unused]] auto const assertion = [&] {
          auto const* numericalNodeAfterStep =
              fEnvironment.GetUniverse()->GetContainingNode(vParticle.GetPosition());
          C8LOG_TRACE(fmt::format(
              "Geometry check: numericalNodeAfterStep={} currentLogicalNode={}",
              fmt::ptr(numericalNodeAfterStep), fmt::ptr(currentLogicalNode)));
          return numericalNodeAfterStep == currentLogicalNode;
        };

        assert(assertion()); // numerical and logical nodes don't match
      } else {               // boundary crossing, step is limited by volume boundary
        vParticle.SetNode(nextVol);
        /*
          DoBoundary may delete the particle (or not)

          small caveat: any changes to vParticle, or even the production
          of new secondaries is currently not passed to ParticleCut,
          thus, particles outside the desired phase space may be produced.
        */
        fProcessSequence.DoBoundaryCrossing(vParticle, *currentLogicalNode, *nextVol);
      }
    }

    auto decay(Particle& particle, TStackView& view) {
      C8LOG_DEBUG("decay");
      units::si::InverseTimeType const actual_decay_time =
          fProcessSequence.GetTotalInverseLifetime(particle);

      random::UniformRealDistribution<units::si::InverseTimeType> uniDist(
          actual_decay_time);
      const auto sample_process = uniDist(fRNG);
      units::si::InverseTimeType inv_decay_count = units::si::InverseTimeType::zero();
      auto const returnCode =
          fProcessSequence.SelectDecay(particle, view, sample_process, inv_decay_count);
      SetEventType(view, history::EventType::Decay);
      return returnCode;
    }

    auto interaction(Particle& particle, TStackView& view) {
      C8LOG_DEBUG("collide");

      units::si::InverseGrammageType const current_inv_length =
          fProcessSequence.GetTotalInverseInteractionLength(particle);

      random::UniformRealDistribution<units::si::InverseGrammageType> uniDist(
          current_inv_length);
      const auto sample_process = uniDist(fRNG);
      auto inv_lambda_count = units::si::InverseGrammageType::zero();
      auto const returnCode = fProcessSequence.SelectInteraction(
          particle, view, sample_process, inv_lambda_count);
      SetEventType(view, history::EventType::Interaction);
      return returnCode;
    }

    void SetEventType(TStackView& view, [[maybe_unused]] history::EventType eventType) {
      if constexpr (TStackView::has_event) {
        for (auto&& sec : view) { sec.GetEvent()->setEventType(eventType); }
      }
    }

    // but this here temporarily. Should go into dedicated file later:
    const char* c8_ascii_ =
        R"V0G0N(
  ,ad8888ba,     ,ad8888ba,    88888888ba    ad88888ba   88  88      a8P          db              ad88888ba   
 d8"'    `"8b   d8"'    `"8b   88      "8b  d8"     "8b  88  88    ,88'          d88b            d8"     "8b  
d8'            d8'        `8b  88      ,8P  Y8,          88  88  ,88"           d8'`8b           Y8a     a8P  
88             88          88  88aaaaaa8P'  `Y8aaaaa,    88  88,d88'           d8'  `8b           "Y8aaa8P"   
88             88          88  88""""88'      `"""""8b,  88  8888"88,         d8YaaaaY8b          ,d8"""8b,   
Y8,            Y8,        ,8P  88    `8b            `8b  88  88P   Y8b       d8""""""""8b        d8"     "8b  
 Y8a.    .a8P   Y8a.    .a8P   88     `8b   Y8a     a8P  88  88     "88,    d8'        `8b       Y8a     a8P  
  `"Y8888Y"'     `"Y8888Y"'    88      `8b   "Y88888P"   88  88       Y8b  d8'          `8b       "Y88888P"
	)V0G0N";
  };

} // namespace corsika::cascade
