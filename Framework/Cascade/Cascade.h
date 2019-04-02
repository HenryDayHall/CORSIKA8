
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */

#ifndef _include_corsika_cascade_Cascade_h_
#define _include_corsika_cascade_Cascade_h_

#include <corsika/environment/Environment.h>
#include <corsika/process/ProcessReturn.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/stack/SecondaryView.h>
#include <corsika/units/PhysicalUnits.h>

#include <corsika/setup/SetupTrajectory.h>

/*  see Issue 161, we need to include SetupStack only because we need
    to globally define StackView. This is clearly not nice and should
    be changed, when possible. It might be that StackView needs to be
    templated in Cascade, but this would be even worse... so we don't
    do that until it is really needed.
 */
#include <corsika/setup/SetupStack.h>

#include <cmath>
#include <iostream>

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
   * TrackingInterface providing the functions: <code>void
   * Init();</code> and <code>auto GetTrack(Particle const& p)</auto>,
   * where the latter has a return type of <code>
   * geometry::Trajectory<corsika::geometry::Line or Helix> </code>
   *
   * <b>TProcessList</b> must be a ProcessSequence.
   *            TimeOfIntersection(corsika::geometry::Line const& line,
   *
   * <b>Stack</b> is the storage object for particle data, i.e. with
   * Particle class type <code>Stack::ParticleType</code>
   *
   *
   */

  template <typename TTracking, typename TProcessList, typename TStack>
  class Cascade {
    using Particle = typename TStack::ParticleType;

    // we only want fully configured objects
    Cascade() = delete;

  public:
    /**
     * Cascade class cannot be default constructed, but needs a valid
     * list of physics processes for configuration at construct time.
     */
    Cascade(corsika::environment::Environment const& env, TTracking& tr, TProcessList& pl,
            TStack& stack)
        : fEnvironment(env)
        , fTracking(tr)
        , fProcessSequence(pl)
        , fStack(stack) {}

    /**
     * The Init function is called before the actual cascade simulations.
     * All components of the Cascade simulation must be configured here.
     */
    void Init() {
      fTracking.Init();
      fProcessSequence.Init();
      fStack.Init();
    }

    /**
     * The Run function is the main simulation loop, which processes
     * particles from the Stack until the Stack is empty.
     */
    void Run() {
      while (!fStack.IsEmpty()) {
        while (!fStack.IsEmpty()) {
          auto pNext = fStack.GetNextParticle();
          Step(pNext);
        }
        // do cascade equations, which can put new particles on Stack,
        // thus, the double loop
        // DoCascadeEquations();
      }
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
      setup::Trajectory step = fTracking.GetTrack(vParticle);

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          fProcessSequence.GetTotalInverseInteractionLength(vParticle, step);

      // sample random exponential step length in grammage
      std::exponential_distribution expDist(total_inv_lambda * (1_g / (1_m * 1_m)));
      GrammageType const next_interact = (1_g / (1_m * 1_m)) * expDist(fRNG);

      std::cout << "total_inv_lambda=" << total_inv_lambda
                << ", next_interact=" << next_interact << std::endl;

      // convert next_step from grammage to length
      auto const* currentNode =
          fEnvironment.GetUniverse()->GetContainingNode(vParticle.GetPosition());

      if (currentNode == &*fEnvironment.GetUniverse()) {
        throw std::runtime_error("particle entered void universe");
      }

      LengthType const distance_interact =
          currentNode->GetModelProperties().ArclengthFromGrammage(step, next_interact);

      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(vParticle, step);
      std::cout << "distance_max=" << distance_max << std::endl;

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          fProcessSequence.GetTotalInverseLifetime(vParticle);

      // sample random exponential decay time
      std::exponential_distribution expDistDecay(total_inv_lifetime * 1_s);
      TimeType const next_decay = 1_s * expDistDecay(fRNG);
      std::cout << "total_inv_lifetime=" << total_inv_lifetime
                << ", next_decay=" << next_decay << std::endl;

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * vParticle.GetMomentum().norm() /
                                        vParticle.GetEnergy() * units::constants::c;

      // take minimum of geometry, interaction, decay for next step
      auto const min_distance =
          std::min({distance_interact, distance_decay, distance_max});

      std::cout << " move particle by : " << min_distance << std::endl;

      // here the particle is actually moved along the trajectory to new position:
      // std::visit(setup::ParticleUpdate<Particle>{vParticle}, step);
      vParticle.SetPosition(step.PositionFromArclength(min_distance));
      // .... also update time, momentum, direction, ...
      vParticle.SetTime(vParticle.GetTime() + min_distance / units::constants::c);

      step.LimitEndTo(min_distance);

      // particle.GetNode(); // previous VolumeNode
      vParticle.SetNode(
          currentNode); // NOTE @Max : here we need to distinguish: IF particle step is
      // limited by tracking (via fTracking.GetTrack()), THEN we need
      // to check/update VolumeNodes. In all other cases it is
      // guaranteed that we are still in the same volume

      // apply all continuous processes on particle + track
      process::EProcessReturn status = fProcessSequence.DoContinuous(vParticle, step);

      if (status == process::EProcessReturn::eParticleAbsorbed) {
        std::cout << "Cascade: delete absorbed particle " << vParticle.GetPID() << " "
                  << vParticle.GetEnergy() / 1_GeV << "GeV" << std::endl;
        vParticle.Delete();
        return;
      }

      std::cout << "sth. happening before geometric limit ? "
                << ((min_distance < distance_max) ? "yes" : "no") << std::endl;

      /*
        Create SecondaryView object on Stack. The data container
        remains untouched and identical, and 'projectil' is identical
        to 'vParticle' above this line. However,
        projectil.AddSecondaries populate the SecondaryView, which can
        then be used afterwards for further processing. Thus: it is
        important to use projectle (and not vParticle) for Interaction,
        and Decay!
       */
      setup::StackView secondaries(vParticle);
      [[maybe_unused]] auto projectile = secondaries.GetProjectile();

      if (min_distance < distance_max) { // interaction to happen within geometric limit
        // check whether decay or interaction limits this step

        if (min_distance == distance_interact) {
          std::cout << "collide" << std::endl;

          InverseGrammageType const actual_inv_length =
              fProcessSequence.GetTotalInverseInteractionLength(vParticle, step);

          random::UniformRealDistribution<InverseGrammageType> uniDist(actual_inv_length);
          const auto sample_process = uniDist(fRNG);
          InverseGrammageType inv_lambda_count = 0. * meter * meter / gram;
          fProcessSequence.SelectInteraction(vParticle, projectile, step, sample_process,
                                             inv_lambda_count);
        } else {
          std::cout << "decay" << std::endl;
          InverseTimeType const actual_decay_time =
              fProcessSequence.GetTotalInverseLifetime(vParticle);

          random::UniformRealDistribution<InverseTimeType> uniDist(actual_decay_time);
          const auto sample_process = uniDist(fRNG);
          InverseTimeType inv_decay_count = 0 / second;
          fProcessSequence.SelectDecay(vParticle, projectile, sample_process,
                                       inv_decay_count);
        }

        fProcessSequence.DoSecondaries(secondaries);
        vParticle.Delete(); // last thing in Step function
      }
    }

  private:
    environment::Environment const& fEnvironment;
    TTracking& fTracking;
    TProcessList& fProcessSequence;
    TStack& fStack;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
  };

} // namespace corsika::cascade

#endif
