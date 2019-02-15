
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
#include <corsika/random/ExponentialDistribution.h>
#include <corsika/random/RNGManager.h>
#include <corsika/random/UniformRealDistribution.h>
#include <corsika/setup/SetupTrajectory.h>
#include <corsika/units/PhysicalUnits.h>

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
   * <b>Tracking</b> must be a class according to the
   * TrackingInterface providing the functions:
   * <code>auto GetTrack(Particle const& p)</auto>,
   * with the return type <code>geometry::Trajectory<corsika::geometry::Line>
   * </code>
   *
   * <b>ProcessList</b> must be a ProcessSequence.   *
   * <b>Stack</b> is the storage object for particle data, i.e. with
   * Particle class type <code>Stack::ParticleType</code>
   *
   *

   */

  template <typename Tracking, typename ProcessList, typename Stack>
  class Cascade {
    using Particle = typename Stack::ParticleType;

    // we only want fully configured objects
    Cascade() = delete;

  public:
    /**
     * Cascade class cannot be default constructed, but needs a valid
     * list of physics processes for configuration at construct time.
     */
    Cascade(corsika::environment::Environment const& env, Tracking& tr, ProcessList& pl,
            Stack& stack)
        : fEnvironment(env)
        , fTracking(tr)
        , fProcessSequence(pl)
        , fStack(stack) {}

    /**
     * The Init function is called before the actual cascade simulations.
     * All components of the Cascade simulation must be configured here.
     */
    void Init() {
      fProcessSequence.Init();
      fStack.Init();
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

        std::cout << "initial node " << p.GetNode() << std::endl;
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
    void Step(Particle& particle) {
      using namespace corsika::units::si;

      // determine geometric tracking
      auto [step, geomMaxLength, nextVol] = fTracking.GetTrack(particle);

      // determine combined total interaction length (inverse)
      InverseGrammageType const total_inv_lambda =
          fProcessSequence.GetTotalInverseInteractionLength(particle, step);

      // sample random exponential step length in grammage
      corsika::random::ExponentialDistribution expDist(1 / total_inv_lambda);
      GrammageType const next_interact = expDist(fRNG);

      std::cout << "total_inv_lambda=" << total_inv_lambda
                << ", next_interact=" << next_interact << std::endl;

      auto const* currentLogicalNode = particle.GetNode();

      auto const* currentNumericalNode =
          fEnvironment.GetUniverse()->GetContainingNode(particle.GetPosition());

      std::cout << "nodes: " << currentLogicalNode << " " << currentNumericalNode
                << std::endl;

      if (currentNumericalNode != currentLogicalNode) {
        throw std::runtime_error("numerical and logical nodes don't match");
      }

      if (currentNumericalNode == &*fEnvironment.GetUniverse()) {
        throw std::runtime_error("particle entered void Universe");
      }

      // convert next_step from grammage to length
      LengthType const distance_interact =
          currentLogicalNode->GetModelProperties().ArclengthFromGrammage(step,
                                                                         next_interact);

      // determine the maximum geometric step length
      LengthType const distance_max = fProcessSequence.MaxStepLength(particle, step);
      std::cout << "distance_max=" << distance_max << std::endl;

      // determine combined total inverse decay time
      InverseTimeType const total_inv_lifetime =
          fProcessSequence.GetTotalInverseLifetime(particle);

      // sample random exponential decay time
      corsika::random::ExponentialDistribution expDistDecay(1 / total_inv_lifetime);
      TimeType const next_decay = expDistDecay(fRNG);
      std::cout << "total_inv_lifetime=" << total_inv_lifetime
                << ", next_decay=" << next_decay << std::endl;

      // convert next_decay from time to length [m]
      LengthType const distance_decay = next_decay * particle.GetMomentum().norm() /
                                        particle.GetEnergy() *
                                        corsika::units::constants::c;

      // take minimum of geometry, interaction, decay for next step
      auto const min_distance =
          std::min({distance_interact, distance_decay, distance_max, geomMaxLength});

      std::cout << " move particle by : " << min_distance << std::endl;

      // here the particle is actually moved along the trajectory to new position:
      // std::visit(corsika::setup::ParticleUpdate<Particle>{particle}, step);
      particle.SetPosition(step.PositionFromArclength(min_distance));
      // .... also update time, momentum, direction, ...

      step.LimitEndTo(min_distance);

      // apply all continuous processes on particle + track
      corsika::process::EProcessReturn status =
          fProcessSequence.DoContinuous(particle, step, fStack);

      if (status == corsika::process::EProcessReturn::eParticleAbsorbed) {
        std::cout << "Cascade: delete absorbed particle " << particle.GetPID() << " "
                  << particle.GetEnergy() / 1_GeV << "GeV" << std::endl;
        particle.Delete();
        return;
      }

      std::cout << "sth. happening before geometric limit ? "
                << ((min_distance < geomMaxLength) ? "yes" : "no") << std::endl;

      if (min_distance < geomMaxLength) { // interaction to happen within geometric limit
        if (min_distance == distance_interact) {
          std::cout << "collide" << std::endl;

          InverseGrammageType const actual_inv_length =
              fProcessSequence.GetTotalInverseInteractionLength(particle, step);

          corsika::random::UniformRealDistribution<InverseGrammageType> uniDist(
              actual_inv_length);
          const auto sample_process = uniDist(fRNG);
          InverseGrammageType inv_lambda_count = 0. * meter * meter / gram;
          fProcessSequence.SelectInteraction(particle, step, fStack, sample_process,
                                             inv_lambda_count);
        } else if (min_distance == distance_decay) {
          std::cout << "decay" << std::endl;
          InverseTimeType const actual_decay_time =
              fProcessSequence.GetTotalInverseLifetime(particle);

          corsika::random::UniformRealDistribution<InverseTimeType> uniDist(
              actual_decay_time);
          const auto sample_process = uniDist(fRNG);
          InverseTimeType inv_decay_count = 0 / second;
          fProcessSequence.SelectDecay(particle, fStack, sample_process, inv_decay_count);
        } else { // step-length limitation within volume
          std::cout << "step-length limitation" << std::endl;
        }
      } else { // boundary crossing
        std::cout << "boundary crossing! next node = " << nextVol << std::endl;
        particle.SetNode(nextVol);
        fProcessSequence.DoBoundaryCrossing(particle, *currentLogicalNode, *nextVol);
      }
    }

  private:
    corsika::environment::Environment const& fEnvironment;
    Tracking& fTracking;
    ProcessList& fProcessSequence;
    Stack& fStack;
    corsika::random::RNG& fRNG =
        corsika::random::RNGManager::GetInstance().GetRandomStream("cascade");
  };

} // namespace corsika::cascade

#endif
